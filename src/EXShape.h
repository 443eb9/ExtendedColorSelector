#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>

class EXPrimaryChannelRing
{
public:
    float boundaryDiameter() const;
    float marginedBoundaryDiameter() const;
    float getRingValue(QPointF widgetCoord) const;
    QPointF getWidgetCoord(float value) const;

    float margin;
    float thickness;
    float rotationOffset;
};

class EXChannelPlaneShape
{
public:
    virtual bool
    widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered, QPointF &shapeCoord, const EXPrimaryChannelRing &ring) = 0;
    virtual QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) = 0;

    QPointF shapeToWidgetCoord(const QPointF &shapeCoord, const EXPrimaryChannelRing &ring)
    {
        QPointF widget = shapeToWidgetCoordCentered(shapeCoord, ring);
        widget.setY(-widget.y());
        return widget * 0.5 + QPointF(0.5, 0.5);
    }

    bool widgetToShapeCoord(const QPointF &widgetCoord, QPointF &shapeCoord, const EXPrimaryChannelRing &ring)
    {
        QPointF widgetCoordCentered = (widgetCoord - QPointF(0.5, 0.5)) * 2;
        widgetCoordCentered.setY(-widgetCoordCentered.y());
        return widgetCenteredToShapeCoord(widgetCoordCentered, shapeCoord, ring);
    }
};

class EXSquareChannelPlaneShape : public EXChannelPlaneShape
{
public:
    bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                            QPointF &shapeCoord,
                            const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXTriangleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                            QPointF &shapeCoord,
                            const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXCircleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                            QPointF &shapeCoord,
                            const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

#endif // EXTENDED_SHAPE_H
