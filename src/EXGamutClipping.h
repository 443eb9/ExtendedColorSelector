#ifndef EXGAMUTCLIPPING_H
#define EXGAMUTCLIPPING_H

#include <QPair>
#include <QVector2D>

#include "EXColorModel.h"

class EXGamutClipping
{
public:
    static EXGamutClipping *instance();

    EXGamutClipping();
    QVector2D
    mapAxesToLimited(ColorModelId colorModel, int primary, float primaryValue, QVector2D axes);
    QVector2D
    unmapAxesFromLimited(ColorModelId colorModel, int primary, float primaryValue, QVector2D limited);
    static int getColorModelOffset(ColorModelId colorModel);

private:
    QVector<float> m_limits;

    QPair<QVector2D, QVector2D> getAxesLimitsInterpolated(ColorModelId colorModel, int primary, float primaryValue);
    QPair<QVector2D, QVector2D> getAxesLimits(ColorModelId colorModel, int primary, int primaryValue);
};

#endif // EXGAMUTCLIPPING_H
