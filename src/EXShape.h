#ifndef EXTENDED_SHAPE_H
#define EXTENDED_SHAPE_H

#include <QPointF>
#include <QVector>

class EXPrimaryChannelRing
{
public:
    EXPrimaryChannelRing()
        : margin(0)
        , thickness(0)
        , rotationOffset(0)
        , reversed(false)
    {
    }

    EXPrimaryChannelRing(float margin, float thickness, float rotationOffset, bool reversed)
        : margin(margin)
        , thickness(thickness)
        , rotationOffset(rotationOffset)
        , reversed(reversed)
    {
    }

    float boundaryDiameter() const;
    float marginedBoundaryDiameter() const;
    float getRingValue(QPointF widgetCoord) const;
    QPointF getWidgetCoord(float value) const;

    float margin;
    float thickness;
    float rotationOffset;
    bool reversed;
};

enum EXChannelPlaneShapeId {
    Square = 0,
    Triangle = 1,
    Circle = 2,
};

class EXChannelPlaneShape
{
public:
    virtual QString displayName() const = 0;

    QPointF shapeToWidgetCentered(const QPointF &shapeCoord);
    bool widgetCenteredToShape(const QPointF &widgetCoordCentered, QPointF &shapeCoord);
    QPointF shapeToWidget01(const QPointF &shapeCoord);
    bool widget01ToShape(const QPointF &widgetCoord, QPointF &shapeCoord);
    void updateTransform(bool reverseX, bool reverseY, float rotation, bool swapAxes);

    void setRing(const EXPrimaryChannelRing &ring)
    {
        m_ring = ring;
    }

    const EXPrimaryChannelRing &ring() const
    {
        return m_ring;
    }

protected:
    virtual bool widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered, QPointF &shapeCoord) = 0;
    virtual QPointF shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered) = 0;
    EXPrimaryChannelRing m_ring;

private:
    bool m_reverseX;
    bool m_reverseY;
    bool m_swapAxes;
    float m_rotation;
    float m_rotCos;
    float m_rotSin;
};

class EXSquareChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Square";
    }

    bool widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered, QPointF &shapeCoord) override;
    QPointF shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered) override;
};

class EXTriangleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Triangle";
    }

    bool widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered, QPointF &shapeCoord) override;
    QPointF shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered) override;
};

class EXCircleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Circle";
    }

    bool widgetCenteredToShapeUntransformed(const QPointF &widgetCoordCentered, QPointF &shapeCoord) override;
    QPointF shapeToWidgetCenteredUntransformed(const QPointF &shapeCoordCentered) override;
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
