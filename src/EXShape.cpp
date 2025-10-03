#include <QVector2D>
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

float EXPrimaryChannelRing::getRingValue(QPointF widgetCoord) const
{
    float x = widgetCoord.x() * 2 - 1;
    float y = -(widgetCoord.y() * 2 - 1);
    float t = (atan2f(y, x) + rotationOffset) / M_PI * 0.5 + 0.5;
    float v = t - floorf(t);
    if (reversed) {
        v = 1 - v;
    }

    return v;
}

QPointF EXPrimaryChannelRing::getWidgetCoord(float value) const
{
    if (reversed) {
        value = 1 - value;
    }
    value = (value + 0.5) * 2 * M_PI - rotationOffset;
    float r = 1 - thickness;
    float x = cosf(value) * r * 0.5 + 0.5;
    float y = -sinf(value) * r * 0.5 + 0.5;
    return QPointF(x, y);
}

const QVector<EXChannelPlaneShapeId> EXShapeFactory::AllShapes = {
    Square,
    Triangle,
    Circle,
};

bool EXSquareChannelPlaneShape::widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                                           QPointF &shapeCoord,
                                                           const EXPrimaryChannelRing &ring)
{
    if (ring.thickness == 0.0f) {
        shapeCoord = widgetCoordCentered * 0.5 + QPointF(0.5, 0.5);
    } else {
        float a = ring.marginedBoundaryDiameter() * 0.7071067812;
        float x = widgetCoordCentered.x() / a;
        float y = widgetCoordCentered.y() / a;
        shapeCoord = QPointF(x * 0.5 + 0.5, y * 0.5 + 0.5);
    }

    return shapeCoord.x() >= 0 && shapeCoord.x() <= 1 && shapeCoord.y() >= 0 && shapeCoord.y() <= 1;
}

QPointF EXSquareChannelPlaneShape::shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered,
                                                              const EXPrimaryChannelRing &ring)
{
    if (ring.thickness == 0.0f) {
        return shapeCoordCentered * 2 - QPointF(1, 1);
    }

    float a = ring.marginedBoundaryDiameter() * 0.7071067812;
    float x = (shapeCoordCentered.x() * 2 - 1) * a;
    float y = (shapeCoordCentered.y() * 2 - 1) * a;
    return QPointF(x, y);
}

bool EXTriangleChannelPlaneShape::widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                                             QPointF &shapeCoord,
                                                             const EXPrimaryChannelRing &ring)
{
    bool result = true;
    QVector2D p = QVector2D(widgetCoordCentered);
    const float RAD_120 = M_PI * 120.0 / 180.0;
    float t = ring.marginedBoundaryDiameter();
    QVector2D v0 = QVector2D(cosf(RAD_120 * 0.0), sinf(RAD_120 * 0.0)) * t;
    QVector2D v1 = QVector2D(cosf(RAD_120 * 1.0), sinf(RAD_120 * 1.0)) * t;
    QVector2D v2 = QVector2D(cosf(RAD_120 * 2.0), sinf(RAD_120 * 2.0)) * t;
    QVector2D vc = (v1 + v2) / 2.0;
    QVector2D vh = vc - v0;
    float a = (v0 - v1).length();
    float h = qMax(vh.length(), 1e-6f);

    float y = QVector2D::dotProduct(p - v0, vh / h) / h;
    QVector2D b = p - (v0 * (1 - y) + v1 * y);
    float x = b.length() / qMax(y * a, 1e-6f);
    if (QVector2D::dotProduct(b, v2 - v1) < 0.0f) {
        x = 0;
        result = false;
    }

    shapeCoord = QPointF(x, y);
    return result && y >= 0 && y <= 1 && x >= 0 && x <= 1;
}

QPointF EXTriangleChannelPlaneShape::shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered,
                                                                const EXPrimaryChannelRing &ring)
{
    float x = shapeCoordCentered.x();
    float y = shapeCoordCentered.y();

    const float RAD_120 = M_PI * 120.0 / 180.0;
    float t = ring.marginedBoundaryDiameter();
    QVector2D v0 = QVector2D(cosf(RAD_120 * 0.0), sinf(RAD_120 * 0.0)) * t;
    QVector2D v1 = QVector2D(cosf(RAD_120 * 1.0), sinf(RAD_120 * 1.0)) * t;
    QVector2D v2 = QVector2D(cosf(RAD_120 * 2.0), sinf(RAD_120 * 2.0)) * t;

    QVector2D p = (v0 * (1 - y) + v1 * y) + (v2 - v1) * y * x;
    return QPointF(p.x(), p.y());
}

bool EXCircleChannelPlaneShape::widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                                           QPointF &shapeCoord,
                                                           const EXPrimaryChannelRing &ring)
{
    float x = widgetCoordCentered.x();
    float y = widgetCoordCentered.y();
    float r = sqrtf(x * x + y * y);
    float a = atan2f(y, x) / M_PI * 0.5 + 0.5;
    shapeCoord = QPointF(r / (ring.marginedBoundaryDiameter()), a);
    return shapeCoord.x() >= 0 && shapeCoord.x() <= 1;
}

QPointF EXCircleChannelPlaneShape::shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered,
                                                              const EXPrimaryChannelRing &ring)
{
    float x = shapeCoordCentered.x();
    float y = shapeCoordCentered.y();
    y = 2 * M_PI * y + M_PI;
    x *= ring.marginedBoundaryDiameter();
    return QPointF(cosf(y) * x, sinf(y) * x);
}
