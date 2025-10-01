#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>

class EXChannelPlaneShape
{
public:
    virtual QPointF widgetToShapeCoord(const QPointF &point) = 0;
    virtual QPointF shapeToWidgetCoord(const QPointF &point) = 0;
};

class EXSquareChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QPointF widgetToShapeCoord(const QPointF &point) override;
    QPointF shapeToWidgetCoord(const QPointF &point) override;
};

#endif // EXTENDED_SHAPE_H
