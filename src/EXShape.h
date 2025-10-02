#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>

class EXPrimaryChannelRing
{
public:
    float boundaryDiameter(float widgetSize) const;
    float marginedBoundaryDiameter(float widgetSize) const;
    float getRingValue(QPointF widgetCoordCentered) const;

    float margin;
    float thickness;
    float rotationOffset;
};

class EXChannelPlaneShape
{
public:
    virtual QPointF widgetToShapeCoord(const QPointF &widgetCoordCentered, float boundaryDiameter = -1) = 0;
    virtual QPointF shapeToWidgetCoord(const QPointF &shapeCoordCentered, float boundaryDiameter = -1) = 0;

    QPointF widgetToShapeCoord01(const QPointF &widgetCoord, float boundaryDiameter = -1)
    {
        QPointF widget = widgetCoord * 2 - QPointF(1, 1);
        QPointF shape = widgetToShapeCoord(widget, boundaryDiameter);
        return shape * 0.5 + QPointF(0.5, 0.5);
    }

    QPointF shapeToWidgetCoord01(const QPointF &shapeCoord, float boundaryDiameter = -1)
    {
        QPointF shape = shapeCoord * 2 - QPointF(1, 1);
        QPointF widget = shapeToWidgetCoord(shape, boundaryDiameter);
        return widget * 0.5 + QPointF(0.5, 0.5);
    }
};

class EXSquareChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QPointF widgetToShapeCoord(const QPointF &widgetCoordCentered, float boundaryDiameter = -1) override;
    QPointF shapeToWidgetCoord(const QPointF &shapeCoordCentered, float boundaryDiameter = -1) override;
};

#endif // EXTENDED_SHAPE_H
