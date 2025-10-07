#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include "EXKoColorConverter.h"

EXColorConverter::EXColorConverter(const KoColorSpace *cs, const ColorModelSP &colorModel)
    : m_colorSpace(cs)
    , m_colorModel(colorModel)
    , m_requiresLinearization(colorModel->requiresLinearization())
{
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

KoColor EXColorConverter::displayChannelsToKoColor(const QVector4D &channels) const
{
    KoColor c(m_colorSpace);
    QVector4D baseValues(channels);
    QVector<float> channelVec(c.colorSpace()->channelCount());

    if (m_isRGBA) {
        if (!m_isLinear && m_requiresLinearization) {
            QVector<qreal> tempVec({baseValues[0], baseValues[1], baseValues[2]});
            if (m_exposureSupported) {
                m_colorSpace->profile()->delinearizeFloatValue(tempVec);
            } else {
                m_colorSpace->profile()->delinearizeFloatValueFast(tempVec);
            }
            baseValues = QVector4D(tempVec[0], tempVec[1], tempVec[2], channels[3]);
        }

        if (m_applyGamma) {
            for (int i = 0; i < 3; i++) {
                baseValues[i] = pow(baseValues[i], 2.2);
            }
        }
    }

    // if (m_exposureSupported) {
    //     baseValues *= m_d->channelMaxValues;
    // }

    for (int i = 0; i < channelVec.size(); i++) {
        channelVec[m_logicalToMemoryPosition[i]] = baseValues[i];
    }

    c.colorSpace()->fromNormalisedChannelsValue(c.data(), channelVec);

    return c;
}

QVector4D EXColorConverter::koColorToDisplayChannels(const KoColor &c) const
{
    QVector<float> channelVec(c.colorSpace()->channelCount());
    m_colorSpace->normalisedChannelsValue(c.data(), channelVec);
    QVector4D channels(0, 0, 0, 0);

    for (int i = 0; i < channelVec.size(); i++) {
        channels[i] = channelVec[m_logicalToMemoryPosition[i]];
    }

    if (m_isRGBA) {
        if (m_applyGamma) {
            for (int i = 0; i < 3; i++) {
                channels[i] = pow(channels[i], 1 / 2.2);
            }
        }
        if (!m_isLinear && m_requiresLinearization) {
            QVector<qreal> temp({channels[0], channels[1], channels[2]});
            m_colorSpace->profile()->linearizeFloatValue(temp);
            channels = QVector4D(temp[0], temp[1], temp[2], channels[3]);
        }
    }

    return channels;
}

const KoColorSpace *EXColorConverter::colorSpace() const
{
    return m_colorSpace;
}

const ColorModelSP EXColorConverter::colorModel() const
{
    return m_colorModel;
}
