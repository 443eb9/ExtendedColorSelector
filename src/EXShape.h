#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>

class Shape
{
public:
    virtual QPointF widgetToShapeCoord(const QPointF &point) = 0;
    virtual QPointF shapeToWidgetCoord(const QPointF &point) = 0;
};

class SquareShape : public Shape
{
public:
    QPointF widgetToShapeCoord(const QPointF &point) override;
    QPointF shapeToWidgetCoord(const QPointF &point) override;
};

#endif // EXTENDED_SHAPE_H
