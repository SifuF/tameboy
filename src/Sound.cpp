#include "Sound.hpp"

#include "Bus.hpp"

#include <cmath>
#include <numbers>
#include <limits>

Sound::Sound(Bus* bus) : m_bus(bus)
{
    m_samples.resize(m_blockSize);
    for (auto& s : m_samples) {
        s = 0;
    }
    initialize(1, m_sampleRate, { sf::SoundChannel::Mono });
}

bool Sound::onGetData(Chunk& data)
{
    static const float F2 = 87.31f;
    static const float G2 = 98.00;
    static const float Gs2 = 103.83;
    static const float C3 = 130.81f;

    static const std::vector<float> keys = {
        174.61f, 185.00f, 196.00f, 207.65f, 220.00f, 233.08f, 246.94f, 261.63f, 277.18f,
        293.66f, 311.13f, 329.63f, 349.23f, 369.99f, 392.00f, 415.30f, 440.00f, 466.16f,
        493.88f, 523.25f, 0.0f
    };

    static const std::vector<int> notes = {
        7, 11, 14, 18, 19, 18, 14, 11, 7, 11, 14, 18, 19, 18, 14, 11, 7, 11, 14, 18, 19, 18, 14, 11, 7, 11, 14, 18, 19, 18, 14, 11,
        3, 7, 10, 14, 15, 14, 10, 7, 3, 7, 10, 14, 15, 14, 10, 7, 3, 7, 10, 14, 15, 14, 10, 7, 3, 7, 10, 14, 15, 14, 10, 7,
        7, 11, 14, 18, 19, 18, 14, 11, 7, 11, 14, 18, 19, 18, 14, 11, 7, 11, 14, 18, 19, 18, 14, 11, 7, 11, 14, 18, 19, 18, 14, 11,
        2, 6, 9, 12, 14, 12, 9, 6, 2, 6, 9, 12, 14, 12, 9, 6, 2, 6, 9, 12, 14, 12, 9, 6, 2, 6, 9, 12, 14, 12, 9, 6
    };

    static int n = 0;
    static int m = 0;
    float gain = 1.0f;
    const size_t decayStart = 5 * m_samples.size() / 4;
    for (size_t t = 0; t < m_samples.size(); ++t) {
        const auto sine = sin(2 * std::numbers::pi * keys[notes[n]] * t / m_sampleRate);
        const auto square = keys[notes[n]] == 0.0f ? 0.0f : (sine > 0.0f ? 1.0f : -1.0f);

        const auto sample = 0.3f * std::numeric_limits<int16_t>::max() * square;

        if (t > decayStart) {
            gain -= float(t - decayStart) / float(m_samples.size() - decayStart);
            if (gain < 0.0f) {
                gain = 0.0f;
            }
        }

        float bassF{};
        const auto split = notes.size() / 4;
        if (n < split) {
            bassF = C3;
        }
        else if (n < 2 * split) {
            bassF = Gs2;
        }
        else if (n < 3 * split) {
            bassF = C3;
        }
        else {
            bassF = G2;
        }
        const auto bassSine = sin(2 * std::numbers::pi * bassF * t / m_sampleRate);
        const auto bassSquare = keys[notes[n]] == 0.0f ? 0.0f : (bassSine > 0.0f ? 1.0f : -1.0f);
        auto bassSample = 0.3f * std::numeric_limits<int16_t>::max() * bassSquare;

        if (m % 2 == 0) {
            bassSample = 0.0f;
        }

        m_samples[t] = gain * (sample + bassSample);
    }

    n++;
    if (n > notes.size() - 1) {
        n = 0;
        m++;
    }


    data.samples = m_samples.data();
    data.sampleCount = m_samples.size();
    return true;
}

void Sound::onSeek(sf::Time timeOffset)
{
    throw std::runtime_error("Audio seek not supported");
}

void Sound::process(uint64_t cycleCounter)
{
    if (cycleCounter % 50000) {
        return;
    }

    // master control
    const auto NR52 = m_bus->read(0xFF26);
    const auto audioOn = static_cast<bool>(NR52 & 0b1000'0000);
    const auto channel4On = static_cast<bool>(NR52 & 0b0000'1000);
    const auto channel3On = static_cast<bool>(NR52 & 0b0000'0100);
    const auto channel2On = static_cast<bool>(NR52 & 0b0000'0010);
    const auto channel1On = static_cast<bool>(NR52 & 0b0000'0001);

    // panning
    const auto NR51 = m_bus->read(0xFF25);
    const auto channel4Left = static_cast<bool>(NR51 & 0b1000'0000);
    const auto channel3Left = static_cast<bool>(NR51 & 0b0100'0000);
    const auto channel2Left = static_cast<bool>(NR51 & 0b0010'0000);
    const auto channel1Left = static_cast<bool>(NR51 & 0b0001'0000);
    const auto channel4Right = static_cast<bool>(NR51 & 0b0000'1000);
    const auto channel3Right = static_cast<bool>(NR51 & 0b0000'0100);
    const auto channel2Right = static_cast<bool>(NR51 & 0b0000'0010);
    const auto channel1Right = static_cast<bool>(NR51 & 0b0000'0001);

    // master volume and VIN panning
    const auto NR50 = m_bus->read(0xFF24);
    const auto vinLeft = static_cast<bool>(NR50 & 0b1000'0000);
    const auto vinRight = static_cast<bool>(NR50 & 0b0000'1000);
    const auto leftVolume = static_cast<uint8_t>((NR50 >> 4) & 0b0000'0111);
    const auto rightVolume = static_cast<uint8_t>(NR50 & 0b0000'0111);

    /// channel 1 - pulse

    // pulse with period sweep
    const auto NR10 = m_bus->read(0xFF10);
    const auto channel1Pace = static_cast<uint8_t>((NR10 >> 4) & 0b0000'0111);
    const auto channel1Direction = static_cast<bool>(NR10 & 0b0000'1000);
    const auto channel1IndividualStep = static_cast<uint8_t>(NR10 & 0b0000'0111);

    // length timer and duty cycle
    const auto NR11 = m_bus->read(0xFF11);
    const auto channel1WaveDuty = static_cast<uint8_t>((NR11 >> 6) & 0b0000'0011);
    const auto channel1InitialLengthTimer = static_cast<uint8_t>(NR11 & 0b011'1111);

    // volume and envelope
    const auto NR12 = m_bus->read(0xFF12);
    const auto channel1InitialVolume = static_cast<uint8_t>((NR12 >> 4) & 0b0000'1111);
    const auto channel1EnvDir = static_cast<bool>(NR12 & 0b0000'1000);
    const auto channel1SweepPace = static_cast<uint8_t>(NR12 & 0b000'0111);

    // period low
    const auto NR13 = m_bus->read(0xFF13);
    const auto channel1LowPeriod = static_cast<uint16_t>(NR13);

    // period high and control
    const auto NR14 = m_bus->read(0xFF14);
    const auto channel1Trigger = static_cast<bool>(NR14 & 0b1000'0000);
    const auto channel1LengthEnable = static_cast<bool>(NR14 & 0b0100'0000);
    const auto channel1HighPeriod = static_cast<uint16_t>(NR14 & 0b000'0111);
    const auto channel1Period = static_cast<uint16_t>((channel1HighPeriod << 8) | channel1LowPeriod);

    /// channel 2 - pulse

    // length timer and duty cycle
    const auto NR21 = m_bus->read(0xFF16);
    const auto channel2WaveDuty = static_cast<uint8_t>((NR21 >> 6) & 0b0000'0011);
    const auto channel2InitialLengthTimer = static_cast<uint8_t>(NR21 & 0b011'1111);

    // volume and envelope
    const auto NR22 = m_bus->read(0xFF17);
    const auto channel2InitialVolume = static_cast<uint8_t>((NR22 >> 4) & 0b0000'1111);
    const auto channel2EnvDir = static_cast<bool>(NR22 & 0b0000'1000);
    const auto channel2SweepPace = static_cast<uint8_t>(NR22 & 0b000'0111);

    // period low
    const auto NR23 = m_bus->read(0xFF18);
    const auto channel2LowPeriod = static_cast<uint16_t>(NR23);

    // period high and control
    const auto NR24 = m_bus->read(0xFF19);
    const auto channel2Trigger = static_cast<bool>(NR24 & 0b1000'0000);
    const auto channel2LengthEnable = static_cast<bool>(NR24 & 0b0100'0000);
    const auto channel2HighPeriod = static_cast<uint16_t>(NR24 & 0b000'0111);
    const auto channel2Period = static_cast<uint16_t>((channel2HighPeriod << 8) | channel2LowPeriod);

    // channel 3 - wave

    // DAC enable
    const auto NR30 = m_bus->read(0xFF1A);
    const auto dacEnable = static_cast<bool>(NR30 & 0b1000'0000);

    // length timer
    const auto NR31 = m_bus->read(0xFF1B);
    const auto channel3InitialLengthTimer = static_cast<uint8_t>(NR31);

    // output level
    const auto NR32 = m_bus->read(0xFF1B);
    const auto channel3OutputLevel = static_cast<uint8_t>((NR32 >> 5) & 0b0000'0011);
    auto testSample = 0;
    switch (channel3OutputLevel) {
        case 0: { testSample = 0; break; }
        case 1: { break; }
        case 2: { testSample >>= 1; break; }
        case 3: { testSample >>= 2; break; }
        default: { throw std::runtime_error("Unknown output level"); }
    }

    // period low
    const auto NR33 = m_bus->read(0xFF1D);
    const auto channel3LowPeriod = static_cast<uint16_t>(NR33);

    // period high and control
    const auto NR34 = m_bus->read(0xFF1E);
    const auto channel3Trigger = static_cast<bool>(NR34 & 0b1000'0000);
    const auto channel3LengthEnable = static_cast<bool>(NR34 & 0b0100'0000);
    const auto channel3HighPeriod = static_cast<uint16_t>(NR34 & 0b000'0111);
    const auto channel3Period = static_cast<uint16_t>((channel3HighPeriod << 8) | channel3LowPeriod);

    // wave pattern ram
    // FF30 to FF3F = 16 bytes = 32 samples

    // channel 4 - noise

    // length timer
    const auto NR41 = m_bus->read(0xFF20);
    const auto channel4InitialLengthTimer = static_cast<uint8_t>(NR41 & 0b011'1111);

    // volume and envelope
    const auto NR42 = m_bus->read(0xFF21);
    const auto channel4InitialVolume = static_cast<uint8_t>((NR42 >> 4) & 0b0000'1111);
    const auto channel4EnvDir = static_cast<bool>(NR42 & 0b0000'1000);
    const auto channel4SweepPace = static_cast<uint8_t>(NR42 & 0b000'0111);

    // period low
    const auto NR43 = m_bus->read(0xFF22);
    const auto channel4ClockShift = static_cast<uint8_t>((NR43 >> 4) & 0b0000'1111);
    const auto channel4LfsrWidth = static_cast<bool>(NR43 & 0b0000'1000);
    const auto channel4ClockDivider = static_cast<uint8_t>(NR43 & 0b000'0111);

    // period high and control
    const auto NR44 = m_bus->read(0xFF23);
    const auto channel4Trigger = static_cast<bool>(NR44 & 0b1000'0000);
    const auto channel4LengthEnable = static_cast<bool>(NR44 & 0b0100'0000);
}