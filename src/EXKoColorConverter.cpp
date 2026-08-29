#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include "EXKoColorConverter.h"

EXKoColorConverter::EXKoColorConverter(const KoColorSpace *cs)
    : m_colorSpace(cs)
    , m_colorModel(ColorModelFactory::fromKoColorSpace(cs))
{
    if (m_colorModel) {
        m_colorModel->setProfile(cs->profile());
    }

    const QList<KoChannelInfo *> channelList = cs->channels();

    for (int i = 0; i < channelList.size(); i++) {
        const KoChannelInfo *channel = channelList.at(i);
        quint32 logical = channel->displayPosition();
        m_logicalToMemoryPosition[logical] = i;
    }

    if ((cs->colorDepthId() == Float16BitsColorDepthID || cs->colorDepthId() == Float32BitsColorDepthID
         || cs->colorDepthId() == Float64BitsColorDepthID)
        && cs->colorModelId() != LABAColorModelID && cs->colorModelId() != CMYKAColorModelID) {
        m_exposureSupported = true;
    } else {
        m_exposureSupported = false;
    }
    m_isRGBA = (cs->colorModelId() == RGBAColorModelID);

    const KoColorProfile *profile = cs->profile();
    m_isLinear = (profile && profile->isLinear());

    if (m_isRGBA) {
        m_applyGamma = m_isLinear;
    }
}

KoColor EXKoColorConverter::displayChannelsToKoColor(const QVector4D &channels) const
{
    KoColor c(m_colorSpace);
    QVector<float> channelVec(m_colorSpace->channelCount());
    displayChannelsToKoColor(c.data(), channels, channelVec);
    return c;
}

void EXKoColorConverter::displayChannelsToKoColor(quint8 *target,
                                                  const QVector4D &channels,
                                                  QVector<float> &tempChannelBuffer) const
{
    QVector4D baseValues(channels);

    for (int i = 0; i < tempChannelBuffer.size(); i++) {
        tempChannelBuffer[m_logicalToMemoryPosition[i]] = baseValues[i];
    }

    m_colorSpace->fromNormalisedChannelsValue(target, tempChannelBuffer);
}

QVector4D EXKoColorConverter::koColorToDisplayChannels(const KoColor &c) const
{
    QVector<float> channelVec(c.colorSpace()->channelCount());
    return koColorToDisplayChannels(c, channelVec);
}

QVector4D EXKoColorConverter::koColorToDisplayChannels(const KoColor &c, QVector<float> &tempChannelBuffer) const
{
    m_colorSpace->normalisedChannelsValue(c.data(), tempChannelBuffer);
    QVector4D channels(0, 0, 0, 0);

    for (int i = 0; i < tempChannelBuffer.size(); i++) {
        channels[i] = tempChannelBuffer[m_logicalToMemoryPosition[i]];
    }

    return channels;
}

const KoColorSpace *EXKoColorConverter::colorSpace() const
{
    return m_colorSpace;
}

const ColorModelSP EXKoColorConverter::colorModel() const
{
    return m_colorModel;
}
