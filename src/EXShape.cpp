#include <cmath>
#include <qmath.h>

#include "EXShape.h"

float EXPrimaryChannelRing::boundaryDiameter() const
{
    return 1 - thickness * 2;
}

float EXPrimaryChannelRing::marginedBoundaryDiameter() const
{
    return 1 - (margin + thickness) * 2;
}

float EXPrimaryChannelRing::getRingValue(QPointF widgetCoordCentered) const
{
    float x = (atan2f(widgetCoordCentered.y(), widgetCoordCentered.x()) + rotationOffset) / M_PI * 0.5 + 0.5;
    return x - floorf(x);
}

QPointF EXSquareChannelPlaneShape::widgetToShapeCoord(const QPointF &widgetCoordCentered, const EXPrimaryChannelRing &ring)
{
    if (ring.thickness == 0.0f) {
        return widgetCoordCentered;
    }

    float a = ring.marginedBoundaryDiameter() * 0.7071067812;
    float x = widgetCoordCentered.x() / a;
    float y = widgetCoordCentered.y() / a;
    return QPointF(x, y);
}

QPointF EXSquareChannelPlaneShape::shapeToWidgetCoord(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring)
{
    if (ring.thickness == 0.0f) {
        return shapeCoordCentered;
    }

    float a = ring.marginedBoundaryDiameter() * 0.7071067812;
    float x = shapeCoordCentered.x() * a;
    float y = shapeCoordCentered.y() * a;
    return QPointF(x, y);
}
