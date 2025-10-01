#ifndef EXTENDEDCOLOR_H
#define EXTENDEDCOLOR_H

#include <array>

#include <QVector3D>

#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>

typedef KisSharedPtr<class ColorModel> ColorModelSP;

enum ColorModelId {
    Rgb = 0,
    Hsv = 1,
    Hsl = 2,
    Oklab = 3,
    Oklch = 4,
};

class ColorModel : public KisShared
{
public:
    virtual QVector3D toXyz(const QVector3D &color) const = 0;
    virtual QVector3D fromXyz(const QVector3D &color) const = 0;
    virtual ColorModelId id() const = 0;
    virtual QString displayName() const = 0;
    virtual std::array<QString, 3> channelNames() const = 0;
    virtual std::array<QPair<qreal, qreal>, 3> channelRanges() const;
};

class RGBModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

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
        return {QPair(0, 100), QPair(0, 100), QPair(0, 100)};
    }
};

class HSVModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Hsv;
    }

    QString displayName() const override
    {
        return "HSV";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"H", "S", "V"};
    }

    std::array<QPair<qreal, qreal>, 3> channelRanges() const override
    {
        return {QPair(0, 360), QPair(0, 100), QPair(0, 100)};
    }
};

class HSLModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Hsl;
    }

    QString displayName() const override
    {
        return "HSL";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"H", "S", "L"};
    }

    std::array<QPair<qreal, qreal>, 3> channelRanges() const override
    {
        return {QPair(0, 360), QPair(0, 100), QPair(0, 100)};
    }
};

class OKLABModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Oklab;
    }

    QString displayName() const override
    {
        return "OkLAB";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"L", "A", "B"};
    }

    std::array<QPair<qreal, qreal>, 3> channelRanges() const override
    {
        return {QPair(0, 100), QPair(-100, 100), QPair(100, 100)};
    }
};

class OKLCHModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Oklch;
    }

    QString displayName() const override
    {
        return "OkLCH";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"L", "C", "H"};
    }

    std::array<QPair<qreal, qreal>, 3> channelRanges() const override
    {
        return {QPair(0, 100), QPair(0, 100), QPair(0, 360)};
    }
};

class ColorModelFactory
{
public:
    static ColorModel *fromId(ColorModelId id)
    {
        switch (id) {
        case ColorModelId::Rgb:
            return new RGBModel();
        case ColorModelId::Hsv:
            return new HSVModel();
        case ColorModelId::Hsl:
            return new HSLModel();
        case ColorModelId::Oklab:
            return new OKLABModel();
        case ColorModelId::Oklch:
            return new OKLCHModel();
        default:
            return nullptr;
        }
    }

    static ColorModel *fromKoColorSpace(const KoColorSpace *colorSpace)
    {
        auto id = colorSpace->colorModelId();
        if (id == RGBAColorModelID) {
            return new RGBModel();
        } else {
            // TODO handle this.
            return 0;
        }
    }

    static const ColorModelId AllModels[5];
};

#endif
