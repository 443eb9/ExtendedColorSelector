#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>
#include <QVector>

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

enum EXChannelPlaneShapeId {
    Square = 0,
    Triangle = 1,
    Circle = 2,
};

class EXChannelPlaneShape
{
public:
    virtual bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                            QPointF &shapeCoord,
                                            const EXPrimaryChannelRing &ring) = 0;
    virtual QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) = 0;
    virtual QString displayName() const = 0;

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
    QString displayName() const override
    {
        return "Square";
    }

    bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                    QPointF &shapeCoord,
                                    const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXTriangleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Triangle";
    }

    bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                    QPointF &shapeCoord,
                                    const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXCircleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Circle";
    }

    bool widgetCenteredToShapeCoord(const QPointF &widgetCoordCentered,
                                    QPointF &shapeCoord,
                                    const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCoordCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXShapeFactory
{
public:
    static const QVector<EXChannelPlaneShapeId> AllShapes;

    static EXChannelPlaneShape *fromId(EXChannelPlaneShapeId id)
    {
        switch (id) {
        case Square:
            return new EXSquareChannelPlaneShape();
        case Triangle:
            return new EXTriangleChannelPlaneShape();
        case Circle:
            return new EXCircleChannelPlaneShape();
        default:
            return nullptr;
        }
    }

    static EXChannelPlaneShape *fromName(const QString &name)
    {
        for (EXChannelPlaneShapeId id : AllShapes) {
            EXChannelPlaneShape *shape = fromId(id);
            if (shape && shape->displayName() == name) {
                return shape;
            }
            delete shape;
        }
        return nullptr;
    }
};

#endif // EXTENDED_SHAPE_H
