#ifndef EXTENDEDCOLOR_H
#define EXTENDEDCOLOR_H

#include <array>

#include <QVector3D>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

typedef KisSharedPtr<class ColorModel> ColorModelSP;

enum ColorModelId {
    Rgb = 0,
};

class ColorModel : public KisShared
{
public:
    virtual QVector3D toXyz(const QVector3D &color) = 0;
    virtual QVector3D fromXyz(const QVector3D &color) = 0;
    virtual ColorModelId id() const = 0;
    virtual QString displayName() const = 0;
    virtual std::array<QString, 3> channelNames() const = 0;
    virtual std::array<QPair<qreal, qreal>, 3> channelRanges() const;
};

class RGBModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) override;
    QVector3D fromXyz(const QVector3D &color) override;

    ColorModelId id() const override
    {
        return ColorModelId::Rgb;
    }

    QString displayName() const override
    {
        return "RGB";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"R", "G", "B"};
    }

    std::array<QPair<qreal, qreal>, 3> channelRanges() const override
    {
        return {QPair(0, 1), QPair(0, 1), QPair(0, 1)};
    }
};

class ColorModelFactory
{
public:
    static ColorModel *toModel(ColorModelId id)
    {
        switch (id) {
        case ColorModelId::Rgb:
            return new RGBModel();
        default:
            return nullptr;
        }
    }
};

#endif
