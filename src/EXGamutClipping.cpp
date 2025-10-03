#include <QDebug>
#include <QFile>
#include <QVector>
#include <qmath.h>

#include "EXGamutClipping.h"

const int Segments = 256;

static EXGamutClipping *s_gamutClipping = nullptr; 

EXGamutClipping *EXGamutClipping::instance()
{
    if (!s_gamutClipping) {
        s_gamutClipping = new EXGamutClipping();
    }
    return s_gamutClipping;
}

EXGamutClipping::EXGamutClipping()
{
    QFile file(":/extendedcolorselector/axes_limits.bytes");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open axes_limits.bytes:" << file.errorString();
        return;
    }

    QByteArray limitsBytes = file.readAll();
    file.close();

    QVector<float> limits;
    const int numFloats = limitsBytes.size() / sizeof(float);
    limits.resize(numFloats);
    memcpy(limits.data(), limitsBytes.constData(), limitsBytes.size());

    m_limits = limits;
}

int EXGamutClipping::getColorModelOffset(ColorModelId colorModel)
{
    switch (colorModel) {
    case ColorModelId::Xyz:
        return 0;
    case ColorModelId::Lab:
        return 1;
    case ColorModelId::Lch:
        return 2;
    case ColorModelId::Oklab:
        return 3;
    case ColorModelId::Oklch:
        return 4;
    default:
        return -1;
    }
}

QVector2D EXGamutClipping::unmapAxesFromLimited(ColorModelId colorModel, int primary, float primaryValue, QVector2D axes)
{
    auto [minLimits, maxLimits] = getAxesLimitsInterpolated(colorModel, primary, primaryValue);

    return QVector2D((axes.x() - minLimits.x()) / (maxLimits.x() - minLimits.x()),
                     (axes.y() - minLimits.y()) / (maxLimits.y() - minLimits.y()));
}

QVector2D
EXGamutClipping::mapAxesToLimited(ColorModelId colorModel, int primary, float primaryValue, QVector2D limited)
{
    auto [minLimits, maxLimits] = getAxesLimitsInterpolated(colorModel, primary, primaryValue);

    return QVector2D(limited.x() * (maxLimits.x() - minLimits.x()) + minLimits.x(),
                     limited.y() * (maxLimits.y() - minLimits.y()) + minLimits.y());
}

QPair<QVector2D, QVector2D>
EXGamutClipping::getAxesLimitsInterpolated(ColorModelId colorModel, int primary, float primaryValue)
{
    float a = primaryValue * Segments;
    auto [min1, max1] = getAxesLimits(colorModel, primary, qFloor(a));
    auto [min2, max2] = getAxesLimits(colorModel, primary, qCeil(a));
    int t = a - int(a);

    return {
        QVector2D(min1.x() * (1 - t) + min2.x() * t, min1.y() * (1 - t) + min2.y() * t),
        QVector2D(max1.x() * (1 - t) + max2.x() * t, max1.y() * (1 - t) + max2.y() * t),
    };
}

QPair<QVector2D, QVector2D> EXGamutClipping::getAxesLimits(ColorModelId colorModel, int primary, int primaryValue)
{
    int offset = getColorModelOffset(colorModel);
    if (offset < 0) {
        return {QVector2D(0, 0), QVector2D(1, 1)};
    }

    int base = (offset * 3 + primary) * (Segments + 1) + primaryValue;
    base *= 4;
    return {
        QVector2D(m_limits[base + 0], m_limits[base + 1]),
        QVector2D(m_limits[base + 2], m_limits[base + 3]),
    };
}
