#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include "EXKoColorConverter.h"

EXColorConverter::EXColorConverter(const KoColorSpace *cs)
    : m_colorSpace(cs)
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

const int *EXColorConverter::displayToMemoryPositionMapper() const
{
    return m_logicalToMemoryPosition;
}

KoColor EXColorConverter::displayChannelsToKoColor(const QVector4D &channels) const
{
    KoColor c(m_colorSpace);
    QVector4D baseValues(channels);
    QVector<float> channelVec(c.colorSpace()->channelCount());

    if (m_isRGBA) {
        if (!m_isLinear) {
            // Note: not all profiles define a TRC necessary for (de-)linearization,
            // substituting with a linear profiles would be better
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

    for (int i = 0; i < 4; i++) {
        channelVec[m_logicalToMemoryPosition[i]] = baseValues[i];
    }

    c.colorSpace()->fromNormalisedChannelsValue(c.data(), channelVec);

    return c;
}

QVector4D EXColorConverter::koColorToDisplayChannels(const KoColor &c) const
{
    QVector<float> channelVec(c.colorSpace()->channelCount());
    m_colorSpace->normalisedChannelsValue(c.data(), channelVec);
    QVector4D channelValuesDisplay(0, 0, 0, 0), coordinates(0, 0, 0, 0);

    for (int i = 0; i < 4; i++) {
        channelValuesDisplay[i] = channelVec[m_logicalToMemoryPosition[i]];
    }

    if (m_isRGBA) {
        if (m_applyGamma) {
            for (int i = 0; i < 3; i++) {
                channelValuesDisplay[i] = pow(channelValuesDisplay[i], 1 / 2.2);
            }
        }
        if (!m_isLinear) {
            QVector<qreal> temp({channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2]});
            m_colorSpace->profile()->linearizeFloatValue(temp);
            channelValuesDisplay = QVector4D(temp[0], temp[1], temp[2], channelValuesDisplay[3]);
        }
    } else {
        for (int i = 0; i < 4; i++) {
            coordinates[i] = channelValuesDisplay[i];
        }
    }
    return coordinates;
}
