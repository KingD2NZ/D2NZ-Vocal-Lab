#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace d2nz {
struct Settings {
    float clean = 0, power = 0, deess = 0, space = 0, echo = 0, tone = 0;
    float outputDb = 0;
};
struct Meters { float inputPeak = 0, outputPeak = 0, reductionDb = 0; };

// Initial prototype: zero-lookahead chain. No pitch correction or denoising.
// prepare() owns allocations; process() is bounded and allocation-free.
class VocalChain {
public:
    void prepare(double sampleRate) {
        rate = sampleRate;
        for (auto& channel : channels) {
            channel = Channel{};
            channel.echo.assign(static_cast<size_t>(rate * 0.24) + 1, 0.f);
            channel.room.assign(static_cast<size_t>(rate * (&channel == &channels[0] ? 0.071 : 0.079)) + 1, 0.f);
        }
        smoothing = static_cast<float>(1.0 - std::exp(-1.0 / (0.02 * rate)));
        current = target = Settings{};
    }
    void reset() noexcept {
        for (auto& c : channels) {
            std::fill(c.echo.begin(), c.echo.end(), 0.f);
            std::fill(c.room.begin(), c.room.end(), 0.f);
            c.echoIndex = c.roomIndex = 0;
            c.hpInput = c.hpOutput = c.low = c.sibilanceLow = c.envelope = c.sibilanceEnvelope = c.roomDamp = 0;
        }
        current = target;
    }
    void setSettings(Settings value) noexcept {
        value.clean = unit(value.clean); value.power = unit(value.power);
        value.deess = unit(value.deess); value.space = unit(value.space);
        value.echo = unit(value.echo); value.tone = unit(value.tone);
        value.outputDb = std::clamp(value.outputDb, -24.f, 12.f);
        target = value;
    }
    Meters process(float* const* audio, int count, int samples) noexcept {
        Meters m;
        if (count < 1 || count > 2 || channels[0].echo.empty()) return m;
        const float highpass = static_cast<float>(std::exp(-2.0 * pi * 90.0 / rate));
        const float lowpass = static_cast<float>(1.0 - std::exp(-2.0 * pi * 700.0 / rate));
        const float sibilance = static_cast<float>(1.0 - std::exp(-2.0 * pi * 5500.0 / rate));
        const float attack = coefficient(0.008), release = coefficient(0.12);
        const float essAttack = coefficient(0.001), essRelease = coefficient(0.05);
        for (int n = 0; n < samples; ++n) {
            smooth(current.clean, target.clean); smooth(current.power, target.power);
            smooth(current.deess, target.deess); smooth(current.space, target.space);
            smooth(current.echo, target.echo); smooth(current.tone, target.tone);
            smooth(current.outputDb, target.outputDb);
            std::array<float, 2> values{};
            float detector = 0;
            for (int ch = 0; ch < count; ++ch) {
                auto& c = channels[static_cast<size_t>(ch)];
                const float input = audio[ch][n];
                m.inputPeak = std::max(m.inputPeak, std::abs(input));
                const float hp = highpass * (c.hpOutput + input - c.hpInput);
                c.hpInput = input; c.hpOutput = hp;
                const float cleaned = input + current.clean * (hp - input);
                c.low += lowpass * (cleaned - c.low);
                values[static_cast<size_t>(ch)] = cleaned - 0.25f * current.clean * c.low;
                detector = std::max(detector, std::abs(values[static_cast<size_t>(ch)]));
            }
            // Linked stereo detector avoids channel-dependent image movement.
            auto& envelope = channels[0].envelope;
            envelope = follow(envelope, detector, attack, release);
            const float levelDb = 20.f * std::log10(std::max(envelope, 1.e-9f));
            const float threshold = -12.f - 18.f * current.power;
            const float ratio = 1.f + 4.f * current.power;
            const float gr = std::max(0.f, levelDb - threshold) * (1.f - 1.f / ratio);
            const float gain = dbGain(-gr);
            m.reductionDb = std::max(m.reductionDb, gr);
            for (int ch = 0; ch < count; ++ch) {
                auto& c = channels[static_cast<size_t>(ch)];
                float x = values[static_cast<size_t>(ch)] * gain;
                c.sibilanceLow += sibilance * (x - c.sibilanceLow);
                const float high = x - c.sibilanceLow;
                c.sibilanceEnvelope = follow(c.sibilanceEnvelope, std::abs(high), essAttack, essRelease);
                const float essGain = 1.f / (1.f + current.deess * 20.f * c.sibilanceEnvelope);
                x = c.sibilanceLow + high * essGain;
                // TONE blends soft harmonic colour; zero is transparent.
                x += current.tone * (std::tanh(2.f * x) * 0.5f - x);
                const float room = c.room[c.roomIndex];
                c.roomDamp += 0.25f * (room - c.roomDamp);
                c.room[c.roomIndex] = x + 0.72f * c.roomDamp;
                c.roomIndex = (c.roomIndex + 1) % c.room.size();
                const float echo = c.echo[c.echoIndex];
                c.echo[c.echoIndex] = x + 0.35f * echo;
                c.echoIndex = (c.echoIndex + 1) % c.echo.size();
                const float out = (x + 0.2f * current.space * room + 0.3f * current.echo * echo) * dbGain(current.outputDb);
                audio[ch][n] = out;
                m.outputPeak = std::max(m.outputPeak, std::abs(out));
            }
        }
        return m;
    }
private:
    struct Channel {
        float hpInput=0, hpOutput=0, low=0, sibilanceLow=0, envelope=0, sibilanceEnvelope=0, roomDamp=0;
        std::vector<float> echo, room;
        size_t echoIndex=0, roomIndex=0;
    };
    static constexpr double pi = 3.14159265358979323846;
    static float unit(float x) noexcept { return std::clamp(x, 0.f, 1.f); }
    static float dbGain(float db) noexcept { return std::pow(10.f, db / 20.f); }
    float coefficient(double seconds) const noexcept { return static_cast<float>(std::exp(-1.0 / (rate * seconds))); }
    static float follow(float previous, float value, float attack, float release) noexcept {
        const float k = value > previous ? attack : release;
        return k * previous + (1.f - k) * value;
    }
    void smooth(float& value, float destination) const noexcept { value += smoothing * (destination - value); }
    double rate = 48000;
    float smoothing = 0;
    Settings current, target;
    std::array<Channel, 2> channels;
};
}
