#pragma once

#include <SFML/Audio.hpp>

#include <vector>

class Bus;

class Sound : public sf::SoundStream {
public:
    Sound(Bus* bus);
    void process(uint64_t cycleCounter);

private:
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;

    std::vector<std::int16_t> m_samples{};
    std::size_t m_currentSample{};
    static constexpr uint32_t m_blockSize = 4000;
    static constexpr uint32_t m_sampleRate = 44100;

    Bus* m_bus{};
};
