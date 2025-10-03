#include <QVector2D>
#include <cmath>
#include <qmath.h>
#include <qdebug.h>

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

QPointF EXChannelPlaneShape::shapeToWidgetCentered(const QPointF &shapeCoord,
                                                              const EXPrimaryChannelRing &ring)
{
    QPointF shape = shapeCoord;
    if (m_swapAxes) {
        shape = QPointF(shape.y(), shape.x());
    }
    if (m_reverseX) {
        shape.setX(1 - shape.x());
    }
    if (m_reverseY) {
        shape.setY(1 - shape.y());
    }

    QPointF widget = shapeToWidgetCenteredUntransformed(shape, ring);
    if (m_rotation == 0.0f) {
        return widget;
    }

    float x = widget.x() * m_rotCos - widget.y() * m_rotSin;
    float y = widget.x() * m_rotSin + widget.y() * m_rotCos;
    return QPointF(x, y);
}

bool EXChannelPlaneShape::widgetCenteredToShape(const QPointF &widgetCoordCentered,
                                                           QPointF &shapeCoord,
                                                           const EXPrimaryChannelRing &ring)
{
    QPointF widget = widgetCoordCentered;
    if (m_rotation != 0.0f) {
        float x = widgetCoordCentered.x() * m_rotCos - widgetCoordCentered.y() * (-m_rotSin);
        float y = widgetCoordCentered.x() * (-m_rotSin) + widgetCoordCentered.y() * m_rotCos;
        widget = QPointF(x, y);
    }

    bool result = widgetCenteredToShapeUntransformed(widget, shapeCoord, ring);
    if (m_reverseX) {
        shapeCoord.setX(1 - shapeCoord.x());
    }
    if (m_reverseY) {
        shapeCoord.setY(1 - shapeCoord.y());
    }
    if (m_swapAxes) {
        shapeCoord = QPointF(shapeCoord.y(), shapeCoord.x());
    }

    return result;
}

QPointF EXChannelPlaneShape::shapeToWidget01(const QPointF &shapeCoord, const EXPrimaryChannelRing &ring)
{
    QPointF widget = shapeToWidgetCentered(shapeCoord, ring);
    widget.setY(-widget.y());
    return widget * 0.5 + QPointF(0.5, 0.5);
}

bool EXChannelPlaneShape::widget01ToShape(const QPointF &widgetCoord,
                                                     QPointF &shapeCoord,
                                                     const EXPrimaryChannelRing &ring)
{
    QPointF widgetCoordCentered = (widgetCoord - QPointF(0.5, 0.5)) * 2;
    widgetCoordCentered.setY(-widgetCoordCentered.y());
    return widgetCenteredToShape(widgetCoordCentered, shapeCoord, ring);
}

void EXChannelPlaneShape::updateTransform(bool reverseX, bool reverseY, float rotation, bool swapAxes)
{
    m_reverseX = reverseX;
    m_reverseY = reverseY;
    m_rotation = rotation;
    sincosf(rotation, &m_rotSin, &m_rotCos);
    m_swapAxes = swapAxes;
}

bool EXSquareChannelPlaneShape::widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered,
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

QPointF EXSquareChannelPlaneShape::shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered,
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

bool EXTriangleChannelPlaneShape::widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered,
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

QPointF EXTriangleChannelPlaneShape::shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered,
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

bool EXCircleChannelPlaneShape::widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered,
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

QPointF EXCircleChannelPlaneShape::shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered,
                                                         const EXPrimaryChannelRing &ring)
{
    float x = shapeCoordCentered.x();
    float y = shapeCoordCentered.y();
    y = 2 * M_PI * y + M_PI;
    x *= ring.marginedBoundaryDiameter();
    return QPointF(cosf(y) * x, sinf(y) * x);
}
