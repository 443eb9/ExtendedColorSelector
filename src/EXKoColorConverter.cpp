#include "EXKoColorConverter.h"

EXColorConverter::EXColorConverter(const KoColorSpace *colorSpace)
    : m_colorSpace(colorSpace)
{
    Q_ASSERT(colorSpace->channelCount() < 8);

    const QList<KoChannelInfo *> channelList = colorSpace->channels();
    for (int i = 0; i < channelList.size(); i++) {
        const KoChannelInfo *channel = channelList.at(i);
        quint32 logical = channel->displayPosition();
        m_displayToMemoryPosition[logical] = i;
    }
}

const int *EXColorConverter::displayToMemoryPositionMapper() const
{
    return m_displayToMemoryPosition;
}

KoColor EXColorConverter::displayChannelsToKoColor(const QVector<float> &channels) const
{
    QVector<float> rearranged(channels.size());
    for (int i = 0; i < (int)m_colorSpace->channelCount(); i++) {
        rearranged[m_displayToMemoryPosition[i]] = channels[i];
    }
    KoColor koColor(m_colorSpace);
    m_colorSpace->fromNormalisedChannelsValue(koColor.data(), rearranged);
    return koColor;
}

QVector<float> EXColorConverter::koColorToDisplayChannels(const KoColor &color) const
{
    QVector<float> memory(m_colorSpace->channelCount());
    m_colorSpace->normalisedChannelsValue(color.data(), memory);
    QVector<float> displayed(m_colorSpace->channelCount());
    const QList<KoChannelInfo *> channelList = m_colorSpace->channels();
    for (int i = 0; i < channelList.size(); i++) {
        const KoChannelInfo *channel = channelList.at(i);
        quint32 logical = channel->displayPosition();
        displayed[logical] = memory[i];
    }

    return displayed;
}
