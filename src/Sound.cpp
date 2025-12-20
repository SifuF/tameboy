#include "Sound.hpp"

void Stream::load(const sf::SoundBuffer& buffer)
{
    m_samples.assign(buffer.getSamples(), buffer.getSamples() + buffer.getSampleCount());
    m_currentSample = 0;

    initialize(buffer.getChannelCount(), buffer.getSampleRate(),
        { sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight });
}

bool Stream::onGetData(Chunk& data)
{
    const int samplesToStream = 50000;
    data.samples = &m_samples[m_currentSample];

    if (m_currentSample + samplesToStream <= m_samples.size()) {
        data.sampleCount = samplesToStream;
        m_currentSample += samplesToStream;
        return true;
    }
    else {
        data.sampleCount = m_samples.size() - m_currentSample;
        m_currentSample = m_samples.size();
        return false;
    }
}

void Stream::onSeek(sf::Time timeOffset)
{
    m_currentSample = static_cast<std::size_t>(timeOffset.asSeconds() * getSampleRate() * getChannelCount());
}

void Sound::run()
{
    try {
        const sf::SoundBuffer buffer("../sounds/test.mp3");

        m_stream.load(buffer);
        m_stream.play();

        while (m_stream.getStatus() == Stream::Status::Playing) {
            sf::sleep(sf::seconds(0.1f));
        }
    }
    catch (...) {
        return;
    }
}
