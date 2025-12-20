#include <SFML/Audio.hpp>

#include <vector>

class Stream : public sf::SoundStream {
public:
    void load(const sf::SoundBuffer& buffer);

private:
    bool onGetData(Chunk& data) override;
    void onSeek(sf::Time timeOffset) override;

    std::vector<std::int16_t> m_samples;
    std::size_t m_currentSample{};
};

class Sound {
public:
    void run();

private:
    Stream m_stream{};
};
