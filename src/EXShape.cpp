#include <cmath>
#include <qmath.h>

#include "EXShape.h"

float EXPrimaryChannelRing::boundaryDiameter(float widgetSize) const
{
    if (thickness <= 0) {
        return -1;
    } else {
        return (widgetSize - thickness * 2) / widgetSize;
    }
}

float EXPrimaryChannelRing::marginedBoundaryDiameter(float widgetSize) const
{
    if (thickness <= 0) {
        return -1;
    } else {
        return (widgetSize - (margin + thickness) * 2) / widgetSize;
    }
}

float EXPrimaryChannelRing::getRingValue(QPointF widgetCoordCentered) const
{
    float x = (atan2f(widgetCoordCentered.y(), widgetCoordCentered.x()) + rotationOffset) / M_PI * 0.5 + 0.5;
    return x - floorf(x);
}

QPointF EXSquareChannelPlaneShape::widgetToShapeCoord(const QPointF &widgetCoordCentered, float boundaryDiameter)
{
    if (boundaryDiameter == -1) {
        return widgetCoordCentered;
    }

    float a = boundaryDiameter * 0.7071067812;
    float x = widgetCoordCentered.x() / a;
    float y = widgetCoordCentered.y() / a;
    return QPointF(x, y);
}

QPointF EXSquareChannelPlaneShape::shapeToWidgetCoord(const QPointF &shapeCoordCentered, float boundaryDiameter)
{
    if (boundaryDiameter == -1) {
        return shapeCoordCentered;
    }

    float a = boundaryDiameter * 0.7071067812;
    float x = shapeCoordCentered.x() * a;
    float y = shapeCoordCentered.y() * a;
    return QPointF(x, y);
}
