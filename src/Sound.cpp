#include "Sound.hpp"

#include <cmath>
#include <numbers>
#include <limits>

Sound::Sound()
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
}
