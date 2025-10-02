#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>

class EXPrimaryChannelRing
{
public:
    float boundaryDiameter() const;
    float marginedBoundaryDiameter() const;
    float getRingValue(QPointF widgetCoordCentered) const;

    float margin;
    float thickness;
    float rotationOffset;
};

class EXChannelPlaneShape
{
public:
    virtual QPointF widgetToShapeCoord(const QPointF &widgetCoordCentered, const EXPrimaryChannelRing &ring) = 0;
    virtual QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) = 0;

    QPointF shapeToWidgetCoord(const QPointF &shapeCoord, const EXPrimaryChannelRing &ring)
    {
        QPointF widget = shapeToWidgetCoordCentered(shapeCoord, ring);
        return widget * 0.5 + QPointF(0.5, 0.5);
    }
};

class EXSquareChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QPointF widgetToShapeCoord(const QPointF &widgetCoordCentered, const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

#endif // EXTENDED_SHAPE_H
