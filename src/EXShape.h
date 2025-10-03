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

    QPointF shapeToWidgetCenteredTransformed(const QPointF &shapeCoord, const EXPrimaryChannelRing &ring);
    bool widgetCenteredToShapeTransformed(const QPointF &widgetCoordCentered,
                                               QPointF &shapeCoord,
                                               const EXPrimaryChannelRing &ring);

    QPointF shapeToWidget01Transformed(const QPointF &shapeCoord, const EXPrimaryChannelRing &ring);
    bool
    widget01ToShapeTransformed(const QPointF &widgetCoord, QPointF &shapeCoord, const EXPrimaryChannelRing &ring);
    void updateTransform(bool reverseX, bool reverseY, float rotation, bool swapAxes);

protected:
    virtual bool widgetCenteredToShape(const QPointF &widgetCoordCentered,
                                            QPointF &shapeCoord,
                                            const EXPrimaryChannelRing &ring) = 0;
    virtual QPointF shapeToWidgetCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) = 0;

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

    bool widgetCenteredToShape(const QPointF &widgetCoordCentered,
                                    QPointF &shapeCoord,
                                    const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXTriangleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Triangle";
    }

    bool widgetCenteredToShape(const QPointF &widgetCoordCentered,
                                    QPointF &shapeCoord,
                                    const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
};

class EXCircleChannelPlaneShape : public EXChannelPlaneShape
{
public:
    QString displayName() const override
    {
        return "Circle";
    }

    bool widgetCenteredToShape(const QPointF &widgetCoordCentered,
                                    QPointF &shapeCoord,
                                    const EXPrimaryChannelRing &ring) override;
    QPointF shapeToWidgetCentered(const QPointF &shapeCoordCentered, const EXPrimaryChannelRing &ring) override;
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
