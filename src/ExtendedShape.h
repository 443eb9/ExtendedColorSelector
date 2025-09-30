#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>

class Shape
{
public:
    virtual QPointF widgetToShapeCoordinate(const QPointF &point) = 0;
    virtual QPointF shapeToWidgetCoordinate(const QPointF &point) = 0;
};

class SquareShape : public Shape
{
public:
    QPointF widgetToShapeCoordinate(const QPointF &point) override;
    QPointF shapeToWidgetCoordinate(const QPointF &point) override;
};

#endif // EXTENDED_SHAPE_H
