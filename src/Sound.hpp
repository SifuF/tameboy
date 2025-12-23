#pragma once

#include <SFML/Audio.hpp>

#include <vector>

class Bus;

template<size_t size>
class Channel {
public:
    Channel(Bus* bus) : m_bus(bus) {}

    void tick(uint64_t cycles)
    {
        while (m_timer <= 0)
        {
            if constexpr (size == 32)
            {
                m_timer += (2048 - m_period) * 2;
                m_index = (m_index + 1) & 0b0001'1111;
            }
            else
            {
                m_timer += (2048 - m_period) * 4;
                m_index = (m_index + 1) & 0b0000'0111;
            }
        }
    }

    uint8_t get() { return m_waveform(m_index); }
   
    void setPeriod(uint16_t period) { m_period = period; }
    
    void setWaveform(uint16_t duty)
    { 
        switch (duty) {
            case 0: { m_waveform = std::array<uint8_t, 8>{1, 0, 0, 0, 0, 0, 0, 0}; break; }
            case 1: { m_waveform = std::array<uint8_t, 8>{1, 1, 0, 0, 0, 0, 0, 0}; break; }
            case 2: { m_waveform = std::array<uint8_t, 8>{1, 1, 1, 1, 0, 0, 0, 0}; break; }
            case 3: { m_waveform = std::array<uint8_t, 8>{1, 1, 1, 1, 1, 1, 0, 0}; break; }
            case 4: {
                static_assert(size == 32);
                for (int offset = 0; offset < 16; ++offset) {
                    const auto value = m_bus->read(0xFF30 + offset);
                    const auto msn = static_cast<uint8_t>(value >> 4);
                    const auto lsn = static_cast<uint8_t>(value & 0b0000'1111);
                    m_period[2 * offset] = msn;
                    m_period[2 * offset + 1] = lsn;
                }
            }
            default: { throw std::runtime_error("Invalid duty cycle value"); }
        }
    }

private:
    int32_t m_timer{}; // TODO - check type
    uint16_t m_period{};
    uint8_t m_index{};
    std::array<uint8_t, size> m_waveform{};

    uint8_t m_pace{};
    bool m_direction{};
    uint8_t individualStep{};

    uint8_t m_waveDuty{};
    uint8_t m_initialLengthTimer{};

    uint8_t m_initialVolume{};
    bool m_envDir{};
    uint8_t m_sweepPace{};

    bool m_trigger{};
    bool m_lengthEnable{};

    Bus* m_bus{};
};

class Sound : public sf::SoundStream {
public:
    Sound(Bus* bus);
    void tick(uint64_t cycles);
    void printState();

private:
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;

    std::vector<std::int16_t> m_samples{};
    std::size_t m_currentSample{};
    static constexpr uint32_t m_blockSize = 4000;
    static constexpr uint32_t m_sampleRate = 44100;
   
    Channel<8> m_channel1;
    Channel<8> m_channel2;
    Channel<32> m_channel3;
    Channel<8> m_channel4;
 
    Bus* m_bus{};
};
