#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace lofier
{
constexpr double kPi    = 3.14159265358979323846;
constexpr double kTwoPi = 6.28318530717958647692;

inline double fract(double x) { return x - std::floor(x); }
inline double mix(double a, double b, double t) { return a + (b - a) * t; }
inline double sign(double x) { return (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0); }
inline double genMod(double x, double y) { return (y > 0.0) ? (x - y * std::floor(x / y)) : 0.0; }

struct FastNoise
{
    uint32_t state = 0x12345678u;
    inline double next()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return (static_cast<double>(x) / 2147483647.5) - 1.0; // [-1, 1]
    }
};

class DelayLine
{
public:
    void prepare(int maxSamples)
    {
        size = std::max(maxSamples, 16) + 8;
        buffer.assign(static_cast<size_t>(size), 0.0);
        writeIdx = 0;
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0);
        writeIdx = 0;
    }

    inline void write(double sample)
    {
        buffer[static_cast<size_t>(writeIdx)] = sample;
        if (++writeIdx >= size) writeIdx = 0;
    }

    inline double readNone(double delaySamples) const
    {
        double pos = static_cast<double>(writeIdx - 1) - delaySamples;
        while (pos < 0.0) pos += static_cast<double>(size);
        int idx = static_cast<int>(pos) % size;
        return buffer[static_cast<size_t>(idx)];
    }

    inline double readLinear(double delaySamples) const
    {
        double pos = static_cast<double>(writeIdx - 1) - delaySamples;
        while (pos < 0.0) pos += static_cast<double>(size);
        int i0 = static_cast<int>(pos) % size;
        int i1 = (i0 + 1) % size;
        double frac = pos - std::floor(pos);
        return mix(buffer[static_cast<size_t>(i0)], buffer[static_cast<size_t>(i1)], frac);
    }

    inline double readCubic(double delaySamples) const
    {
        double pos = static_cast<double>(writeIdx - 1) - delaySamples;
        while (pos < 0.0) pos += static_cast<double>(size);
        int i1 = static_cast<int>(pos) % size;
        double frac = pos - std::floor(pos);
        int i0 = (i1 - 1 + size) % size;
        int i2 = (i1 + 1) % size;
        int i3 = (i1 + 2) % size;

        double y0 = buffer[static_cast<size_t>(i0)];
        double y1 = buffer[static_cast<size_t>(i1)];
        double y2 = buffer[static_cast<size_t>(i2)];
        double y3 = buffer[static_cast<size_t>(i3)];

        double c0 = y1;
        double c1 = 0.5 * (y2 - y0);
        double c2 = y0 - 2.5 * y1 + 2.0 * y2 - 0.5 * y3;
        double c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);
        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

private:
    std::vector<double> buffer;
    int size = 64;
    int writeIdx = 0;
};

// Internal Envelope & Transient Tracker
struct EnvTransientTracker
{
    double env = 0.0, fast = 0.0, slow = 0.0;
    void reset() { env = fast = slow = 0.0; }

    inline void process(double inL, double inR, double sr, double& outEnv, double& outTrans)
    {
        double lvl = std::max(std::abs(inL), std::abs(inR));
        double atk = 1.0 - std::exp(-1.0 / (0.005 * sr));
        double rel = 1.0 - std::exp(-1.0 / (0.120 * sr));
        env += (lvl - env) * ((lvl > env) ? atk : rel);

        double fAtk = 1.0 - std::exp(-1.0 / (0.002 * sr));
        double fRel = 1.0 - std::exp(-1.0 / (0.025 * sr));
        fast += (lvl - fast) * ((lvl > fast) ? fAtk : fRel);

        double sRate = 1.0 - std::exp(-1.0 / (0.180 * sr));
        slow += (lvl - slow) * sRate;

        outEnv   = std::clamp(env * 1.6, 0.0, 1.0);
        outTrans = std::clamp((fast - slow) * 4.5, 0.0, 1.0);
    }
};

// 1. Sub-Anchor LR4 Crossover & Warmth (Mono instance, used x2 for stereo)
struct SubCrossoverMono
{
    double lpf1_z1 = 0, lpf1_z2 = 0, lpf2_z1 = 0, lpf2_z2 = 0;
    double hpf1_z1 = 0, hpf1_z2 = 0, hpf2_z1 = 0, hpf2_z2 = 0;
    double flux_int = 0, dc_x = 0, dc_y = 0;

    void reset()
    {
        lpf1_z1 = lpf1_z2 = lpf2_z1 = lpf2_z2 = 0;
        hpf1_z1 = hpf1_z2 = hpf2_z1 = hpf2_z2 = 0;
        flux_int = dc_x = dc_y = 0;
    }

    inline void process(double in1, double fcIn, double driveIn, double sr, double& outSub, double& outHigh)
    {
        double fc = std::clamp(fcIn, 30.0, 250.0);
        double drive = std::clamp(driveIn, 1.0, 5.0);
        double warm_norm = (drive - 1.0) * 0.25;

        double omega = kTwoPi * fc / sr;
        double sn = std::sin(omega);
        double cs = std::cos(omega);
        double alpha = sn / (2.0 * 0.70710678);
        double a0_inv = 1.0 / (1.0 + alpha);

        double b0_lp = ((1.0 - cs) * 0.5) * a0_inv;
        double b1_lp = (1.0 - cs) * a0_inv;
        double b2_lp = b0_lp;
        double a1 = (-2.0 * cs) * a0_inv;
        double a2 = (1.0 - alpha) * a0_inv;

        double b0_hp = ((1.0 + cs) * 0.5) * a0_inv;
        double b1_hp = -(1.0 + cs) * a0_inv;
        double b2_hp = b0_hp;

        double lp_s1 = b0_lp * in1 + lpf1_z1;
        lpf1_z1 = b1_lp * in1 - a1 * lp_s1 + lpf1_z2;
        lpf1_z2 = b2_lp * in1 - a2 * lp_s1;

        double sub_raw = b0_lp * lp_s1 + lpf2_z1;
        lpf2_z1 = b1_lp * lp_s1 - a1 * sub_raw + lpf2_z2;
        lpf2_z2 = b2_lp * lp_s1 - a2 * sub_raw;

        double hp_s1 = b0_hp * in1 + hpf1_z1;
        hpf1_z1 = b1_hp * in1 - a1 * hp_s1 + hpf1_z2;
        hpf1_z2 = b2_hp * in1 - a2 * hp_s1;

        double high_band = b0_hp * hp_s1 + hpf2_z1;
        hpf2_z1 = b1_hp * hp_s1 - a1 * high_band + hpf2_z2;
        hpf2_z2 = b2_hp * hp_s1 - a2 * high_band;

        double g_flux = std::clamp(1.0 - std::exp(-kTwoPi * 28.0 / sr), 0.0005, 0.05);
        flux_int += g_flux * (std::abs(sub_raw) - flux_int);
        double dyn_drive = 1.0 + (drive - 1.0) * std::clamp(0.55 + flux_int * 1.35, 0.55, 1.45);

        double asym_bias = 0.14 * warm_norm;
        double x_in = sub_raw * dyn_drive + asym_bias;
        double sat_raw = (std::tanh(x_in) - std::tanh(asym_bias)) / std::max(std::tanh(dyn_drive) * 0.92, 0.1);
        double sub_warm = mix(sub_raw, sat_raw, warm_norm);

        double r_dc = std::clamp(1.0 - (kTwoPi * 3.5 / sr), 0.9985, 0.9999);
        dc_y = sub_warm - dc_x + r_dc * dc_y;
        dc_x = sub_warm;

        outSub  = dc_y;
        outHigh = high_band;
    }
};

// 2. Sub Iron & Sub-Octave Thump
struct SubIronModule
{
    double rms_env = 0.05, prev_H_L = 0, prev_H_R = 0, mag_L = 0, mag_R = 0;
    double flux_L = 0, flux_R = 0, fund_lp1 = 0, fund_lp2 = 0;
    double flip_state = 1.0, trig_arm = 1.0, sub_oct_lp1 = 0, sub_oct_lp2 = 0;
    double dc_xL = 0, dc_yL = 0, dc_xR = 0, dc_yR = 0;

    void reset()
    {
        rms_env = 0.05; prev_H_L = prev_H_R = mag_L = mag_R = 0;
        flux_L = flux_R = fund_lp1 = fund_lp2 = 0;
        flip_state = 1.0; trig_arm = 1.0; sub_oct_lp1 = sub_oct_lp2 = 0;
        dc_xL = dc_yL = dc_xR = dc_yR = 0;
    }

    inline void process(double& ioL, double& ioR, double trans, double iron, double thump, double sr)
    {
        double in_L = ioL, in_R = ioR;
        double mono_in = (in_L + in_R) * 0.5;
        double foc_mix = std::clamp(iron * 0.5 + thump * 0.85, 0.0, 1.0);
        double foc_L = mix(in_L, mono_in, foc_mix);
        double foc_R = mix(in_R, mono_in, foc_mix);

        double sq_in = std::max(foc_L * foc_L, foc_R * foc_R);
        rms_env += (sq_in - rms_env) * ((sq_in > rms_env) ? 0.002 : 0.00015);
        double rms_val = std::sqrt(std::max(rms_env, 0.0001));
        double level_gain = mix(1.0, std::clamp(0.28 / std::max(rms_val, 0.14), 0.75, 1.60), iron * 0.70);

        double g_fund = std::clamp(1.0 - std::exp(-kTwoPi * 55.0 / sr), 0.001, 0.5);
        fund_lp1 += g_fund * (mono_in - fund_lp1);
        fund_lp2 += g_fund * (fund_lp1 - fund_lp2);
        double bump_boost = fund_lp2 * iron * 0.52;

        double flux_pole = std::clamp(1.0 - (kTwoPi * 18.0 / sr), 0.98, 0.9995);
        flux_L = flux_L * flux_pole + foc_L * (1.0 - flux_pole) * 3.2;
        flux_R = flux_R * flux_pole + foc_R * (1.0 - flux_pole) * 3.2;

        double H_L = (foc_L + bump_boost + flux_L * iron * 0.42) * level_gain * (1.0 + iron * 1.35);
        double H_R = (foc_R + bump_boost + flux_R * iron * 0.42) * level_gain * (1.0 + iron * 1.35);
        double dH_L = H_L - prev_H_L;
        double dH_R = H_R - prev_H_R;
        prev_H_L = H_L;
        prev_H_R = H_R;

        double rem = 0.24 * iron, coerc = 0.20 * iron, bias = 0.15 * iron;
        double pin_L = sign(dH_L) * coerc * (1.0 - std::exp(-std::abs(dH_L) * 4.5));
        double pin_R = sign(dH_R) * coerc * (1.0 - std::exp(-std::abs(dH_R) * 4.5));

        double tb = std::tanh(bias);
        double targ_L = std::tanh(H_L + bias + (mag_L * rem) - pin_L) - tb;
        double targ_R = std::tanh(H_R + bias + (mag_R * rem) - pin_R) - tb;
        mag_L += (targ_L - mag_L) * (0.72 + std::abs(dH_L) * 0.22);
        mag_R += (targ_R - mag_R) * (0.72 + std::abs(dH_R) * 0.22);

        if (fund_lp2 > 0.0004 && trig_arm < 0.0) { flip_state = -flip_state; trig_arm = 1.0; }
        else if (fund_lp2 < -0.0004 && trig_arm > 0.0) { trig_arm = -1.0; }

        double dyn_thump = std::clamp(0.35 + trans * 0.85, 0.35, 1.2);
        double raw_oct = flip_state * std::abs(fund_lp2) * dyn_thump * 2.4;
        double g_oct = std::clamp(1.0 - std::exp(-kTwoPi * 58.0 / sr), 0.001, 0.5);
        sub_oct_lp1 += g_oct * (raw_oct - sub_oct_lp1);
        sub_oct_lp2 += g_oct * (sub_oct_lp1 - sub_oct_lp2);
        double oct_sig = std::tanh(sub_oct_lp2 * 1.85) * thump * 0.85;

        double raw_out_L = mix(in_L, mag_L, iron) + oct_sig;
        double raw_out_R = mix(in_R, mag_R, iron) + oct_sig;

        double r_dc = std::clamp(1.0 - (kTwoPi * 4.5 / sr), 0.998, 0.9999);
        dc_yL = raw_out_L - dc_xL + r_dc * dc_yL; dc_xL = raw_out_L;
        dc_yR = raw_out_R - dc_xR + r_dc * dc_yR; dc_xR = raw_out_R;

        ioL = dc_yL;
        ioR = dc_yR;
    }
};

// 3. Transport Inertia (Wow, Flutter, Scrape, Wallace Dropouts)
struct TransportModule
{
    DelayLine tape_L, tape_R;
    FastNoise rng;
    double wow_ph = 0, reel_ph = 0, flut_ph = 0;
    double drift_1 = 0, drift_2 = 0, flut_rnd = 0;
    double fly_pos = 0, fly_vel = 0, scr_bp = 0, scr_lp = 0;
    double drop_tmr = 0, drop_targ_L = 0, drop_targ_R = 0, drop_env_L = 0, drop_env_R = 0;
    double wallace_L = 0, wallace_R = 0;

    void prepare(double sr)
    {
        int maxSamps = std::max(44100, static_cast<int>(std::ceil(sr * 1.1)));
        tape_L.prepare(maxSamps);
        tape_R.prepare(maxSamps);
        reset();
    }

    void reset()
    {
        tape_L.reset(); tape_R.reset();
        wow_ph = reel_ph = flut_ph = drift_1 = drift_2 = flut_rnd = 0;
        fly_pos = fly_vel = scr_bp = scr_lp = 0;
        drop_tmr = drop_targ_L = drop_targ_R = drop_env_L = drop_env_R = 0;
        wallace_L = wallace_R = 0;
    }

    inline void process(double& ioL, double& ioR, double wow_amt, double flut_amt, double scrape_amt,
                        double env, double trans, double sr)
    {
        tape_L.write(ioL);
        tape_R.write(ioR);

        double n_w = rng.next();
        drift_1 += (n_w - drift_1) * 0.00045;
        drift_2 += (drift_1 - drift_2) * 0.00018;
        double belt_drift = std::clamp(drift_2 * 48.0, -1.0, 1.0);

        flut_rnd += (rng.next() - flut_rnd) * 0.0035;
        double roller_jit = std::clamp(flut_rnd * 16.0, -1.0, 1.0);

        double torque_load = (trans * 0.85 + env * 0.25) * std::clamp(wow_amt * 0.8 + flut_amt * 0.6, 0.0, 1.0);
        double omega_servo = kTwoPi * 3.2 / sr;
        double fly_acc = omega_servo * (torque_load - fly_pos - 0.55 * fly_vel);
        fly_vel += fly_acc;
        fly_pos = std::clamp(fly_pos + omega_servo * fly_vel, -0.5, 1.5);
        double inertia_dip = fly_pos * 0.0014;

        double wow_freq = std::clamp(1.12 + belt_drift * 0.38, 0.65, 1.75);
        wow_ph = fract(wow_ph + (wow_freq / sr));
        reel_ph = fract(reel_ph + (0.43 / sr));
        double wow_wave = (std::sin(wow_ph * kTwoPi) * 0.72 + std::sin(reel_ph * kTwoPi + belt_drift) * 0.28
                         + 0.18 * std::sin(wow_ph * kTwoPi * 2.0)) * wow_amt * 0.0032;

        double flut_freq = std::clamp(8.6 + roller_jit * 2.4, 5.5, 12.5);
        flut_ph = fract(flut_ph + (flut_freq / sr));
        double flut_wave = (std::sin(flut_ph * kTwoPi) + 0.25 * std::sin(flut_ph * kTwoPi * 2.71))
                         * flut_amt * (1.0 + env * 0.75) * 0.00038;

        double f_scr = 2.0 * std::sin(kPi * 72.0 / sr);
        double scr_in = rng.next() * (0.25 + env * 0.75);
        scr_lp += f_scr * scr_bp;
        double scr_hp = scr_in - scr_lp - 0.18 * scr_bp;
        scr_bp += f_scr * scr_hp;
        double scrape_mod = scr_bp * scrape_amt * 0.00018;

        double base_del = 0.0055 * sr;
        double drift_del = belt_drift * wow_amt * 0.00085;
        double maxLim = std::min(44000.0, sr);
        double total_L = std::clamp(base_del + (wow_wave + flut_wave + inertia_dip + drift_del + scrape_mod) * sr, 8.0, maxLim);
        double total_R = std::clamp(base_del + (wow_wave * 1.015 - flut_wave * 0.92 + inertia_dip * 0.97 + drift_del - scrape_mod * 0.85) * sr, 8.0, maxLim);

        double rd_L = tape_L.readCubic(total_L);
        double rd_R = tape_R.readCubic(total_R);

        drop_tmr -= 1.0;
        if (drop_tmr <= 0.0)
        {
            drop_tmr = std::floor(sr * (0.018 + std::abs(rng.next()) * 0.045));
            double prob = scrape_amt * scrape_amt * 0.28;
            drop_targ_L = (std::abs(rng.next()) < prob) ? std::clamp(scrape_amt * (0.45 + std::abs(rng.next()) * 0.50), 0.0, 0.92) : 0.0;
            drop_targ_R = (std::abs(rng.next()) < prob * 1.18) ? std::clamp(scrape_amt * (0.50 + std::abs(rng.next()) * 0.45), 0.0, 0.94) : (drop_targ_L * 0.65);
        }

        drop_env_L += (drop_targ_L - drop_env_L) * ((drop_targ_L > drop_env_L) ? 0.006 : 0.0008);
        drop_env_R += (drop_targ_R - drop_env_R) * ((drop_targ_R > drop_env_R) ? 0.006 : 0.0008);

        double liftoff_L = std::clamp(drop_env_L + std::abs(scrape_mod) * 900.0 + std::abs(flut_wave) * 350.0, 0.0, 1.0);
        double liftoff_R = std::clamp(drop_env_R + std::abs(scrape_mod) * 900.0 + std::abs(flut_wave) * 350.0, 0.0, 1.0);

        double fc_loss_L = std::clamp(19000.0 * std::pow(0.06, liftoff_L), 900.0, 19500.0);
        double fc_loss_R = std::clamp(19000.0 * std::pow(0.06, liftoff_R), 900.0, 19500.0);
        double g_wL = std::clamp(1.0 - std::exp(-kTwoPi * fc_loss_L / sr), 0.01, 0.99);
        double g_wR = std::clamp(1.0 - std::exp(-kTwoPi * fc_loss_R / sr), 0.01, 0.99);

        wallace_L += g_wL * (rd_L - wallace_L);
        wallace_R += g_wR * (rd_R - wallace_R);

        ioL = wallace_L * (1.0 - drop_env_L * 0.55);
        ioR = wallace_R * (1.0 - drop_env_R * 0.58);
    }
};

// 4. Tape Head (Hysteresis, Self-Erasure, Azimuth, Crosstalk)
struct TapeHeadModule
{
    DelayLine az_L, az_R;
    FastNoise rng;
    double pre_zL = 0, pre_zR = 0, de_zL = 0, de_zR = 0;
    double flux_env = 0, erase_L = 0, erase_R = 0, xtalk_L = 0, xtalk_R = 0;
    double bump_b = 0, bump_l = 0, sub_hpf = 0, weave_ph = 0, weave_drift = 0;
    double prev_H_L = 0, prev_H_R = 0, mag_M_L = 0, mag_M_R = 0;
    double dc_xL = 0, dc_yL = 0, dc_xR = 0, dc_yR = 0;

    void prepare(double sr)
    {
        int sz = std::max(1024, static_cast<int>(std::ceil(sr * 0.05)));
        az_L.prepare(sz); az_R.prepare(sz);
        reset();
    }

    void reset()
    {
        az_L.reset(); az_R.reset();
        pre_zL = pre_zR = de_zL = de_zR = flux_env = erase_L = erase_R = xtalk_L = xtalk_R = 0;
        bump_b = bump_l = sub_hpf = weave_ph = weave_drift = 0;
        prev_H_L = prev_H_R = mag_M_L = mag_M_R = dc_xL = dc_yL = dc_xR = dc_yR = 0;
    }

    inline void process(double& ioL, double& ioR, double sub_sc, double gap_drive, double azimuth, double sr)
    {
        double high_L = ioL, high_R = ioR;

        double f_bump = 2.0 * std::sin(kPi * 78.0 / sr);
        bump_l += f_bump * bump_b;
        double bump_h = sub_sc - bump_l - 0.55 * bump_b;
        bump_b += f_bump * bump_h;

        double sat_sub = std::tanh(bump_l * (1.5 + gap_drive * 2.5));
        sub_hpf += (sat_sub - sub_hpf) * 0.018;
        double chest_harm = (sat_sub - sub_hpf) * gap_drive * 0.34;

        double g_emp = std::clamp(1.0 - std::exp(-kTwoPi * 3180.0 / sr), 0.01, 0.65);
        pre_zL += g_emp * (high_L - pre_zL);
        pre_zR += g_emp * (high_R - pre_zR);
        double emp_boost = 0.35 + gap_drive * 0.95;
        double rec_L = high_L + (high_L - pre_zL) * emp_boost;
        double rec_R = high_R + (high_R - pre_zR) * emp_boost;

        double H_L = (rec_L + chest_harm) * (1.0 + gap_drive * 1.25);
        double H_R = (rec_R + chest_harm) * (1.0 + gap_drive * 1.25);
        double dH_L = H_L - prev_H_L, dH_R = H_R - prev_H_R;
        prev_H_L = H_L; prev_H_R = H_R;

        double coercivity = 0.22 * gap_drive, remanence = 0.20 * gap_drive, pole_bias = 0.09 * gap_drive;
        double pin_L = sign(dH_L) * coercivity * (1.0 - std::exp(-std::abs(dH_L) * 4.5));
        double pin_R = sign(dH_R) * coercivity * (1.0 - std::exp(-std::abs(dH_R) * 4.5));
        double tpb = std::tanh(pole_bias);

        double target_M_L = std::tanh(H_L + pole_bias + (mag_M_L * remanence) - pin_L) - tpb;
        double target_M_R = std::tanh(H_R + pole_bias + (mag_M_R * remanence) - pin_R) - tpb;
        mag_M_L += (target_M_L - mag_M_L) * (0.68 + std::abs(dH_L) * 0.28);
        mag_M_R += (target_M_R - mag_M_R) * (0.68 + std::abs(dH_R) * 0.28);

        de_zL += g_emp * (mag_M_L - de_zL);
        de_zR += g_emp * (mag_M_R - de_zR);
        double de_mix = std::clamp(emp_boost / (1.0 + emp_boost), 0.0, 0.65);
        double repro_L = mix(mag_M_L, de_zL, de_mix);
        double repro_R = mix(mag_M_R, de_zR, de_mix);

        double instant_flux = std::abs(sub_sc) * 1.35 + (std::abs(repro_L) + std::abs(repro_R)) * 0.48;
        flux_env += (instant_flux - flux_env) * ((instant_flux > flux_env) ? 0.045 : 0.0022);

        double erase_fc = std::clamp(18500.0 * std::pow(0.14, std::clamp(flux_env * gap_drive * 1.25, 0.0, 1.0)), 1500.0, 19500.0);
        double g_er = std::clamp(1.0 - std::exp(-kTwoPi * erase_fc / sr), 0.02, 0.98);
        erase_L += g_er * (repro_L - erase_L);
        erase_R += g_er * (repro_R - erase_R);

        az_L.write(erase_L);
        az_R.write(erase_R);

        weave_drift += (rng.next() - weave_drift) * 0.0008;
        weave_ph = fract(weave_ph + ((0.38 + weave_drift * 0.18) / sr));
        double weave = (std::sin(weave_ph * kTwoPi) * 0.5 + 0.5) * azimuth * 0.00016;

        double gap_samps = std::clamp(azimuth * 0.000045 * sr, 0.0, 8.0);
        double az_samps_R = std::clamp((azimuth * 0.00036 + weave) * sr, 0.0, 512.0);

        double gap_L  = az_L.readCubic(gap_samps);
        double main_R = az_R.readCubic(az_samps_R);
        double gap_R  = az_R.readCubic(az_samps_R + gap_samps);

        double final_L = mix(erase_L, (erase_L * 0.72 + gap_L * 0.28), azimuth);
        double final_R = mix(erase_R, (main_R * 0.72 + gap_R * 0.28), azimuth);

        xtalk_L += (final_L - xtalk_L) * 0.14;
        xtalk_R += (final_R - xtalk_R) * 0.14;
        double cap_L = final_L - xtalk_L;
        double cap_R = final_R - xtalk_R;
        double bleed_amt = azimuth * 0.18;

        double sum_L = final_L + (xtalk_R * 0.65 + cap_R * 0.35) * bleed_amt;
        double sum_R = final_R + (xtalk_L * 0.65 + cap_L * 0.35) * bleed_amt;

        double r_dc = std::clamp(1.0 - (kTwoPi * 4.5 / sr), 0.998, 0.9999);
        dc_yL = sum_L - dc_xL + r_dc * dc_yL; dc_xL = sum_L;
        dc_yR = sum_R - dc_xR + r_dc * dc_yR; dc_xR = sum_R;

        ioL = dc_yL;
        ioR = dc_yR;
    }
};

// 5. AD/DA Compander Crunch
struct CompanderModule
{
    FastNoise rng;
    double pre_zL = 0, pre_zR = 0, de_zL = 0, de_zR = 0, ph = 0, hold_L = 0, hold_R = 0;
    double prev_sat_L = 0, prev_sat_R = 0, recon_L = 0, recon_R = 0;
    double adc_env = 0.1, dac_env = 0.1, jit_lp = 0;

    void reset()
    {
        pre_zL = pre_zR = de_zL = de_zR = ph = hold_L = hold_R = 0;
        prev_sat_L = prev_sat_R = recon_L = recon_R = jit_lp = 0;
        adc_env = dac_env = 0.1;
    }

    inline void process(double& ioL, double& ioR, double env_mod, double sr_targetIn,
                        double bitsIn, double emp_amt, double comp_amt, double sr)
    {
        double in_L = ioL, in_R = ioR;
        double sr_target = std::clamp(sr_targetIn, 1500.0, sr);
        double bits = std::clamp(bitsIn, 4.0, 16.0);

        double g_emp = std::clamp(1.0 - std::exp(-kTwoPi * 3180.0 / sr), 0.01, 0.65);
        pre_zL += g_emp * (in_L - pre_zL);
        pre_zR += g_emp * (in_R - pre_zR);
        double emp_boost = emp_amt * 2.1;
        double emp_L = in_L + (in_L - pre_zL) * emp_boost;
        double emp_R = in_R + (in_R - pre_zR) * emp_boost;

        double in_lvl = (std::abs(emp_L) + std::abs(emp_R)) * 0.5;
        adc_env += (in_lvl - adc_env) * ((in_lvl > adc_env) ? 0.016 : 0.00045);
        double adc_gain = mix(1.0, 1.0 / std::pow(std::clamp(adc_env, 0.018, 1.5), 0.56), comp_amt);

        double drv_L = emp_L * adc_gain, drv_R = emp_R * adc_gain;
        double cBias = std::tanh(0.04 * comp_amt);
        double sat_L = std::tanh(drv_L + 0.04 * comp_amt) - cBias;
        double sat_R = std::tanh(drv_R + 0.04 * comp_amt) - cBias;

        jit_lp += (rng.next() - jit_lp) * 0.15;
        double jitter = jit_lp * (1.0 - sr_target / sr) * (0.008 + env_mod * 0.018);
        double step = std::clamp((sr_target / sr) + jitter, 0.01, 1.0);
        ph += step;

        if (ph >= 1.0)
        {
            double over = ph - 1.0;
            double frac = std::clamp(over / step, 0.0, 1.0);
            ph = fract(ph);

            double samp_L = mix(sat_L, prev_sat_L, frac);
            double samp_R = mix(sat_R, prev_sat_R, frac);

            double levels = std::pow(2.0, bits - 1.0);
            double dnl_L = (0.22 * std::sin(samp_L * kTwoPi * 2.0) + 0.12 * std::cos(samp_L * kTwoPi * 7.0)) / levels;
            double dnl_R = (0.22 * std::sin(samp_R * kTwoPi * 2.0) + 0.12 * std::cos(samp_R * kTwoPi * 7.0)) / levels;
            double dith_L = (rng.next() + rng.next()) * 0.16 / levels;
            double dith_R = (rng.next() + rng.next()) * 0.16 / levels;

            hold_L = std::floor((samp_L + dnl_L + dith_L) * levels + 0.5) / levels;
            hold_R = std::floor((samp_R + dnl_R + dith_R) * levels + 0.5) / levels;
        }
        prev_sat_L = sat_L;
        prev_sat_R = sat_R;

        double recon_fc = std::clamp(sr_target * 0.48, 1200.0, 19500.0);
        double g_rec = std::clamp(1.0 - std::exp(-kTwoPi * recon_fc / sr), 0.05, 0.98);
        recon_L += g_rec * (hold_L - recon_L);
        recon_R += g_rec * (hold_R - recon_R);
        double dac_L = mix(hold_L, recon_L, 0.38);
        double dac_R = mix(hold_R, recon_R, 0.38);

        double dac_lvl = (std::abs(dac_L) + std::abs(dac_R)) * 0.5;
        dac_env += (dac_lvl - dac_env) * ((dac_lvl > dac_env) ? 0.010 : 0.00022);
        double dac_gain = mix(1.0, std::pow(std::clamp(dac_env, 0.018, 1.5), 0.62) * (1.0 + comp_amt * 0.32), comp_amt);

        double exp_L = dac_L * dac_gain, exp_R = dac_R * dac_gain;

        de_zL += g_emp * (exp_L - de_zL);
        de_zR += g_emp * (exp_R - de_zR);
        double de_mix = std::clamp(emp_boost / (1.0 + emp_boost), 0.0, 0.82);

        ioL = mix(exp_L, de_zL, de_mix);
        ioR = mix(exp_R, de_zR, de_mix);
    }
};

// 6. Digital Fault Engine (0=Off, 1=DAC Drift, 2=GSM 1-Bar, 3=RAM Rot)
struct FaultEngineModule
{
    DelayLine pkt_L, pkt_R, spec_L, spec_R;
    FastNoise rng;
    double timer = 0, freeze_state = 0, frz_env = 0, read_idx = 0, frame_len = 882, spec_hold = 64;
    double addr_stride = 1, addr_offset = 0, bit_rot_active = 0, gsm_lpL = 0, gsm_lpR = 0, therm_drift = 0;

    void prepare()
    {
        pkt_L.prepare(4096); pkt_R.prepare(4096);
        spec_L.prepare(1024); spec_R.prepare(1024);
        reset();
    }

    void reset()
    {
        pkt_L.reset(); pkt_R.reset(); spec_L.reset(); spec_R.reset();
        timer = freeze_state = frz_env = read_idx = addr_offset = bit_rot_active = gsm_lpL = gsm_lpR = therm_drift = 0;
        frame_len = 882; spec_hold = 64; addr_stride = 1;
    }

    inline double process(double& ioL, double& ioR, double env, int mode, double sr)
    {
        double in_L = ioL, in_R = ioR;
        pkt_L.write(in_L);
        pkt_R.write(in_R);

        if (mode == 0) return 0.0;

        double drop_prob = (mode == 2) ? (0.12 + env * 0.34) : ((mode == 3) ? (0.26 + env * 0.50) : 0.0);
        timer -= 1.0;
        if (timer <= 0.0)
        {
            timer = std::floor(sr * ((mode == 3) ? (0.008 + std::abs(rng.next()) * 0.018) : 0.020));
            freeze_state = (std::abs(rng.next()) < drop_prob) ? 1.0 : 0.0;
            frame_len = std::floor(sr * ((mode == 3) ? (0.004 + std::abs(rng.next()) * 0.024) : 0.020));
            spec_hold = std::floor(22.0 + std::abs(rng.next()) * 135.0);

            double r1 = rng.next(), r2 = rng.next();
            addr_stride = (r1 > 0.35) ? -1.0 : ((r2 < -0.35) ? 2.0 : 1.0);
            addr_offset = (std::abs(rng.next()) > 0.55) ? std::floor(std::abs(rng.next()) * 256.0) : 0.0;
            bit_rot_active = (mode == 3 && std::abs(rng.next()) < (0.35 + env * 0.45)) ? 1.0 : 0.0;
        }

        read_idx = (freeze_state > 0.5) ? genMod(read_idx + addr_stride + frame_len, std::max(frame_len, 16.0)) : 0.0;
        double actual_idx = std::clamp(read_idx + ((mode == 3) ? addr_offset : 0.0), 0.0, 4090.0);
        double frz_L = pkt_L.readNone(actual_idx);
        double frz_R = pkt_R.readNone(actual_idx);

        frz_env += (freeze_state - frz_env) * ((mode == 3) ? 0.85 : 0.08);
        double pkt_out_L = mix(in_L, frz_L, frz_env);
        double pkt_out_R = mix(in_R, frz_R, frz_env);

        therm_drift += (rng.next() - therm_drift) * 0.0005;
        double msb_amt = (mode == 1) ? (0.022 + therm_drift * 0.008) : ((mode == 2) ? 0.015 : 0.045);
        double ladder_L = pkt_out_L + std::tanh(pkt_out_L * 60.0) * msb_amt * (0.25 + env * 0.75);
        double ladder_R = pkt_out_R + std::tanh(pkt_out_R * 60.0) * msb_amt * (0.25 + env * 0.75);

        if (mode == 3 && bit_rot_active > 0.5)
        {
            double cell_L = std::floor(ladder_L * 16.0);
            double cell_R = std::floor(ladder_R * 16.0);
            double flip_L = (genMod(std::abs(cell_L), 2.0) >= 1.0) ? -0.085 : 0.085;
            double flip_R = (genMod(std::abs(cell_R), 2.0) >= 1.0) ? -0.085 : 0.085;
            ladder_L = std::clamp(ladder_L + flip_L * env, -1.0, 1.0);
            ladder_R = std::clamp(ladder_R + flip_R * env, -1.0, 1.0);
        }

        spec_L.write(ladder_L);
        spec_R.write(ladder_R);
        double comb_L = spec_L.readLinear(spec_hold);
        double comb_R = spec_R.readLinear(spec_hold * 1.12);

        double notch_mix = (mode == 1) ? 0.08 : ((mode == 2) ? (0.48 + frz_env * 0.25) : 0.68);
        double hole_L = ladder_L - (comb_L * notch_mix);
        double hole_R = ladder_R - (comb_R * notch_mix);

        double g_gsm = std::clamp(1.0 - std::exp(-kTwoPi * 3800.0 / sr), 0.05, 0.85);
        gsm_lpL += g_gsm * (hole_L - gsm_lpL);
        gsm_lpR += g_gsm * (hole_R - gsm_lpR);

        ioL = (mode == 2) ? mix(hole_L, gsm_lpL, 0.35 + frz_env * 0.55) : hole_L;
        ioR = (mode == 2) ? mix(hole_R, gsm_lpR, 0.35 + frz_env * 0.55) : hole_R;
        return (mode >= 2) ? freeze_state : 0.0;
    }
};

// 7. Living Dust (AC-Bias Hiss + 45/45 Vinyl Physics)
struct LivingDustModule
{
    DelayLine mod_L, mod_R;
    FastNoise rng;
    double pink_L1 = 0, pink_L2 = 0, pink_R1 = 0, pink_R2 = 0, hpf_L = 0, hpf_R = 0;
    double hiss_lpL = 0, hiss_lpR = 0, prev_L = 0, prev_R = 0;
    double tick_bpL = 0, tick_lpL = 0, tick_bpR = 0, tick_lpR = 0;
    double thump_bpL = 0, thump_lpL = 0, thump_bpR = 0, thump_lpR = 0, pinch_env = 0;

    void prepare()
    {
        mod_L.prepare(256); mod_R.prepare(256);
        reset();
    }

    void reset()
    {
        mod_L.reset(); mod_R.reset();
        pink_L1 = pink_L2 = pink_R1 = pink_R2 = hpf_L = hpf_R = 0;
        hiss_lpL = hiss_lpR = prev_L = prev_R = 0;
        tick_bpL = tick_lpL = tick_bpR = tick_lpR = 0;
        thump_bpL = thump_lpL = thump_bpR = thump_lpR = pinch_env = 0;
    }

    inline void process(double& ioL, double& ioR, double env, double halo_amt, double vinyl_amt, double sr)
    {
        double dry_L = ioL, dry_R = ioR;
        double n_L = rng.next(), n_R = rng.next();

        double cutoff_mod = 0.08 + (env * 0.44);
        pink_L1 += (n_L - pink_L1) * cutoff_mod;
        pink_L2 += (pink_L1 - pink_L2) * cutoff_mod;
        pink_R1 += (n_R - pink_R1) * cutoff_mod;
        pink_R2 += (pink_R1 - pink_R2) * cutoff_mod;

        hpf_L += (pink_L2 - hpf_L) * 0.028;
        hpf_R += (pink_R2 - hpf_R) * 0.028;
        double bias_L = pink_L2 - hpf_L;
        double bias_R = pink_R2 - hpf_R;

        double g_nab = std::clamp(1.0 - std::exp(-kTwoPi * 4800.0 / sr), 0.05, 0.85);
        hiss_lpL += g_nab * ((n_L * 0.35 + bias_L * 0.65) - hiss_lpL);
        hiss_lpR += g_nab * ((n_R * 0.35 + bias_R * 0.65) - hiss_lpR);
        double idle_hiss_L = hiss_lpL * (halo_amt * halo_amt) * 0.016;
        double idle_hiss_R = hiss_lpR * (halo_amt * halo_amt) * 0.016;

        mod_L.write(dry_L);
        mod_R.write(dry_R);
        double jitter_samps_L = 2.0 + (bias_L * (0.15 + env * 0.85) * halo_amt * 1.35);
        double jitter_samps_R = 2.0 + (bias_R * (0.15 + env * 0.85) * halo_amt * 1.35);
        double fm_L = mod_L.readCubic(jitter_samps_L);
        double fm_R = mod_R.readCubic(jitter_samps_R);

        double asperity_L = (fm_L * bias_L * 0.48) + (bias_L * env * env * 0.12);
        double asperity_R = (fm_R * bias_R * 0.48) + (bias_R * env * env * 0.12);

        double tape_L = std::tanh((fm_L + asperity_L * halo_amt + idle_hiss_L) * (1.0 + halo_amt * 0.22));
        double tape_R = std::tanh((fm_R + asperity_R * halo_amt + idle_hiss_R) * (1.0 + halo_amt * 0.22));

        double slew_L = std::abs(dry_L - prev_L), slew_R = std::abs(dry_R - prev_R);
        prev_L = dry_L; prev_R = dry_R;
        double groove_stress = std::clamp(0.22 + (slew_L + slew_R) * 3.2 + (env * 0.38), 0.22, 1.0);

        double pop_thresh = 1.0 - (vinyl_amt * groove_stress * 0.014);
        double scratch_thresh = 1.0 - (vinyl_amt * vinyl_amt * groove_stress * 0.0025);

        double wall_A = (std::abs(n_L) > pop_thresh) ? (sign(n_L) * (0.35 + std::abs(n_R) * 0.65) * groove_stress) : 0.0;
        double wall_B = (std::abs(n_R) > pop_thresh) ? (sign(n_R) * (0.35 + std::abs(n_L) * 0.65) * groove_stress) : 0.0;
        double heavy_hit = (std::abs(n_L) > scratch_thresh) ? (sign(n_L) * 1.4) : 0.0;

        double imp_L = wall_A - wall_B * 0.28 + heavy_hit * 0.8;
        double imp_R = wall_B - wall_A * 0.28 + heavy_hit * 0.65;

        double f_tick = 2.0 * std::sin(kPi * 3200.0 / sr);
        tick_lpL += f_tick * tick_bpL;
        tick_bpL += f_tick * (imp_L - tick_lpL - 0.65 * tick_bpL);
        tick_lpR += f_tick * tick_bpR;
        tick_bpR += f_tick * (imp_R - tick_lpR - 0.65 * tick_bpR);

        double f_thump = 2.0 * std::sin(kPi * 135.0 / sr);
        thump_lpL += f_thump * thump_bpL;
        thump_bpL += f_thump * (heavy_hit - thump_lpL - 0.45 * thump_bpL);
        thump_lpR += f_thump * thump_bpR;
        thump_bpR += f_thump * ((heavy_hit * 0.85) - thump_lpR - 0.45 * thump_bpR);

        double trig_pinch = (std::abs(heavy_hit) > 0.1) ? 0.42 : 0.0;
        pinch_env += (trig_pinch - pinch_env) * ((trig_pinch > pinch_env) ? 0.25 : 0.015);
        double pinch_duck = 1.0 - (pinch_env * vinyl_amt);

        ioL = (tape_L * pinch_duck) + (tick_bpL * 0.75 + thump_bpL * 0.45) * vinyl_amt;
        ioR = (tape_R * pinch_duck) + (tick_bpR * 0.75 + thump_bpR * 0.45) * vinyl_amt;
    }
};

// 8. Voltage Starve & Motor Drag
struct VoltageStarveModule
{
    DelayLine drag_L, drag_R;
    double sag_cap = 0, cog_ph = 0, hum_ph = 0, gate_env = 1.0;
    double slew_L = 0, slew_R = 0, dc_xL = 0, dc_yL = 0, dc_xR = 0, dc_yR = 0;

    void prepare(double sr)
    {
        int sz = std::max(44100, static_cast<int>(std::ceil(sr * 1.1)));
        drag_L.prepare(sz); drag_R.prepare(sz);
        reset();
    }

    void reset()
    {
        drag_L.reset(); drag_R.reset();
        sag_cap = cog_ph = hum_ph = slew_L = slew_R = dc_xL = dc_yL = dc_xR = dc_yR = 0;
        gate_env = 1.0;
    }

    inline double process(double& ioL, double& ioR, double env, double starve, double drag_amt, double sr)
    {
        double in_L = ioL, in_R = ioR;
        drag_L.write(in_L);
        drag_R.write(in_R);

        double load_demand = std::clamp((std::abs(in_L) + std::abs(in_R)) * 0.72 + env * 0.65, 0.0, 1.5);
        double cap_target = load_demand * std::clamp(starve * 0.85 + drag_amt * 0.45, 0.0, 1.0);
        sag_cap += (cap_target - sag_cap) * ((cap_target > sag_cap) ? 0.009 : 0.00010);

        double cog_hz = std::clamp(7.6 - (sag_cap * 4.2) - (drag_amt * 1.6), 1.8, 8.0);
        cog_ph = fract(cog_ph + (cog_hz / sr));
        double cog_wave = (std::sin(cog_ph * kTwoPi) * 0.5 + 0.5) + 0.25 * (std::sin(cog_ph * kTwoPi * 3.0) * 0.5 + 0.5);
        double cog_ripple = cog_wave * sag_cap * drag_amt * 0.0024;

        double del_samps = std::clamp((0.001 + sag_cap * drag_amt * 0.015 + cog_ripple) * sr, 4.0, std::min(44000.0, sr));
        double pitch_L = drag_L.readCubic(del_samps);
        double pitch_R = drag_R.readCubic(del_samps * 1.018);

        double v_rail = std::clamp(1.0 - (starve * 0.54) - (sag_cap * 0.38), 0.08, 1.0);
        hum_ph = fract(hum_ph + (100.0 / sr));
        double psrr_ripple = (std::abs(std::sin(hum_ph * kTwoPi)) - 0.636) * sag_cap * starve * 0.028;
        double bias_shift = (1.0 - v_rail) * 0.42 + psrr_ripple;

        double deadzone = starve * (0.04 + sag_cap * 0.14);
        double dz = std::max(deadzone, 0.005);
        double x_notch_L = pitch_L - deadzone * 0.55 * std::tanh(pitch_L / dz);
        double x_notch_R = pitch_R - deadzone * 0.55 * std::tanh(pitch_R / dz);

        double sig_level = std::max(std::abs(pitch_L), std::abs(pitch_R));
        double gate_trig = (sig_level > deadzone * 0.75) ? 1.0 : 0.0;
        gate_env += (gate_trig - gate_env) * ((gate_trig > gate_env) ? 0.065 : 0.0032);

        double bt = std::tanh(bias_shift / v_rail);
        double crush_L = (std::tanh((x_notch_L + bias_shift) / v_rail) - bt) * v_rail;
        double crush_R = (std::tanh((x_notch_R + bias_shift) / v_rail) - bt) * v_rail;

        double max_slew = std::clamp(v_rail * v_rail * 0.45, 0.008, 1.0);
        slew_L += std::clamp(crush_L - slew_L, -max_slew, max_slew);
        slew_R += std::clamp(crush_R - slew_R, -max_slew, max_slew);

        double gMix = mix(1.0, gate_env, starve * 0.85);
        double sputter_L = slew_L * gMix, sputter_R = slew_R * gMix;

        double r_dc = std::clamp(1.0 - (kTwoPi * 4.5 / sr), 0.998, 0.9999);
        dc_yL = sputter_L - dc_xL + r_dc * dc_yL; dc_xL = sputter_L;
        dc_yR = sputter_R - dc_xR + r_dc * dc_yR; dc_xR = sputter_R;

        double wet = std::clamp(starve + drag_amt, 0.0, 1.0);
        double makeup = 1.0 + starve * 0.38;
        ioL = mix(in_L, dc_yL * makeup, wet);
        ioR = mix(in_R, dc_yR * makeup, wet);
        return sag_cap;
    }
};

// 9. CEM3320 4-Pole Ladder & 3D Wood Enclosure
struct CemWoodModule
{
    DelayLine box_L1, box_R1, box_L2, box_R2;
    double s1_L = 0, s2_L = 0, s3_L = 0, s4_L = 0;
    double s1_R = 0, s2_R = 0, s3_R = 0, s4_R = 0;
    double wood_lpL = 0, wood_lpR = 0, cab_hpfL = 0, cab_hpfR = 0;
    double dc_xL = 0, dc_yL = 0, dc_xR = 0, dc_yR = 0;

    void prepare(double sr)
    {
        int sz = std::max(1024, static_cast<int>(std::ceil(sr * 0.02)));
        box_L1.prepare(sz); box_R1.prepare(sz); box_L2.prepare(sz); box_R2.prepare(sz);
        reset();
    }

    void reset()
    {
        box_L1.reset(); box_R1.reset(); box_L2.reset(); box_R2.reset();
        s1_L = s2_L = s3_L = s4_L = s1_R = s2_R = s3_R = s4_R = 0;
        wood_lpL = wood_lpR = cab_hpfL = cab_hpfR = dc_xL = dc_yL = dc_xR = dc_yR = 0;
    }

    inline void process(double& ioL, double& ioR, double trans, double base_fcIn, double resIn, double box_amt, double sr)
    {
        double in_L = ioL, in_R = ioR;
        double base_fc = std::clamp(base_fcIn, 800.0, 18000.0);
        double res = std::clamp(resIn, 0.0, 0.85);

        double dyn_fc = std::clamp(base_fc + (trans * base_fc * 0.65), 800.0, 19200.0);
        double g = std::clamp(1.0 - std::exp(-kTwoPi * dyn_fc / sr), 0.01, 0.94);
        double k = res * 3.75;

        double g2 = g * g, g3 = g2 * g, g4 = g2 * g2, omg = 1.0 - g;
        double S_L = omg * (g3 * s1_L + g2 * s2_L + g * s3_L + s4_L);
        double S_R = omg * (g3 * s1_R + g2 * s2_R + g * s3_R + s4_R);

        double denom = 1.0 / (1.0 + k * g4);
        double u_L = std::tanh(((in_L * (1.0 + k * 0.42)) - k * S_L) * denom);
        double u_R = std::tanh(((in_R * (1.0 + k * 0.42)) - k * S_R) * denom);

        double v1_L = s1_L + g * (u_L - s1_L);
        s1_L = v1_L - 0.08 * (v1_L * v1_L * v1_L) + 0.015 * (v1_L * v1_L);
        double v2_L = s2_L + g * (s1_L - s2_L);
        s2_L = v2_L - 0.08 * (v2_L * v2_L * v2_L);
        double v3_L = s3_L + g * (s2_L - s3_L);
        s3_L = v3_L - 0.08 * (v3_L * v3_L * v3_L);
        s4_L = s4_L + g * (s3_L - s4_L);

        double v1_R = s1_R + g * (u_R - s1_R);
        s1_R = v1_R - 0.08 * (v1_R * v1_R * v1_R) + 0.015 * (v1_R * v1_R);
        double v2_R = s2_R + g * (s1_R - s2_R);
        s2_R = v2_R - 0.08 * (v2_R * v2_R * v2_R);
        double v3_R = s3_R + g * (s2_R - s3_R);
        s3_R = v3_R - 0.08 * (v3_R * v3_R * v3_R);
        s4_R = s4_R + g * (s3_R - s4_R);

        double filt_L = s4_L, filt_R = s4_R;

        double g_cab = std::clamp(1.0 - std::exp(-kTwoPi * 180.0 / sr), 0.005, 0.25);
        cab_hpfL += g_cab * (filt_L - cab_hpfL);
        cab_hpfR += g_cab * (filt_R - cab_hpfR);
        double exc_L = filt_L - cab_hpfL, exc_R = filt_R - cab_hpfR;

        double r1_L = box_L1.readCubic(sr * 0.00143);
        double r2_L = box_L2.readCubic(sr * 0.00317);
        double r1_R = box_R1.readCubic(sr * 0.00161);
        double r2_R = box_R2.readCubic(sr * 0.00289);

        double g_wood = std::clamp(1.0 - std::exp(-kTwoPi * 3400.0 / sr), 0.05, 0.85);
        wood_lpL += g_wood * (r2_L - wood_lpL);
        wood_lpR += g_wood * (r2_R - wood_lpR);

        box_L1.write(exc_L - wood_lpL * 0.34 + wood_lpR * 0.12);
        box_L2.write(r1_L + exc_L * 0.28);
        box_R1.write(exc_R - wood_lpR * 0.34 + wood_lpL * 0.12);
        box_R2.write(r1_R + exc_R * 0.28);

        double wood_L = (filt_L * 0.72) + (r1_L * 0.52) - (wood_lpL * 0.38);
        double wood_R = (filt_R * 0.72) + (r1_R * 0.52) - (wood_lpR * 0.38);

        double mix_L = mix(filt_L, wood_L, box_amt);
        double mix_R = mix(filt_R, wood_R, box_amt);

        double r_dc = std::clamp(1.0 - (kTwoPi * 4.5 / sr), 0.998, 0.9999);
        dc_yL = mix_L - dc_xL + r_dc * dc_yL; dc_xL = mix_L;
        dc_yR = mix_R - dc_xR + r_dc * dc_yR; dc_xR = mix_R;

        ioL = dc_yL;
        ioR = dc_yR;
    }
};
} // namespace lofier
