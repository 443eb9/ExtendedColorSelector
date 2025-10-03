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
    Xyz = 3,
    Lab = 4,
    Lch = 5,
    Oklab = 6,
    Oklch = 7,
    Okhsv = 8,
    Okhsl = 9,
};

class ColorModel : public KisShared
{
public:
    virtual QVector3D toXyz(const QVector3D &color) const = 0;
    virtual QVector3D fromXyz(const QVector3D &color) const = 0;
    virtual void makeColorful(QVector3D &color) const
    {
    }

    virtual ColorModelId id() const = 0;
    virtual QString displayName() const = 0;
    virtual std::array<QString, 3> channelNames() const = 0;
    virtual std::array<QVector3D, 2> channelRanges() const;
    virtual bool isSrgbBased() const;

    virtual QVector3D unnormalize(const QVector3D &normalized)
    {
        auto [mn, mx] = channelRanges();
        return normalized * (mx - mn) + mn;
    }

    virtual QVector3D normalize(const QVector3D &normalized)
    {
        auto [mn, mx] = channelRanges();
        return (normalized + mn) / (mx - mn);
    }

    QVector3D transferTo(const ColorModel *toModel, const QVector3D &color, const QVector3D *reference = nullptr) const;
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

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(100, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return true;
    }
};

class HSVModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void makeColorful(QVector3D &color) const override;

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

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(360, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return true;
    }
};

class HSLModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void makeColorful(QVector3D &color) const override;

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

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(360, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return true;
    }
};

class XYZModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Xyz;
    }

    QString displayName() const override
    {
        return "XYZ";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"X", "Y", "Z"};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(100, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return false;
    }
};

class LABModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Lab;
    }

    QString displayName() const override
    {
        return "LAB";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"L", "A", "B"};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, -100, -100), QVector3D(100, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return false;
    }
};

class LCHModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Lch;
    }

    QString displayName() const override
    {
        return "LCH";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"L", "C", "H"};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(100, 100, 360)};
    }

    bool isSrgbBased() const override
    {
        return false;
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

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, -100, -100), QVector3D(100, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return false;
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

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(100, 100, 360)};
    }

    bool isSrgbBased() const override
    {
        return false;
    }
};

class OKHSVModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void makeColorful(QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Okhsv;
    }

    QString displayName() const override
    {
        return "OkHSV";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"H", "S", "V"};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(360, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return true;
    }
};

class OKHSLModel : public ColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Okhsl;
    }

    QString displayName() const override
    {
        return "OkHSL";
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"H", "S", "L"};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(360, 100, 100)};
    }

    bool isSrgbBased() const override
    {
        return true;
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
        case ColorModelId::Xyz:
            return new XYZModel();
        case ColorModelId::Lab:
            return new LABModel();
        case ColorModelId::Lch:
            return new LCHModel();
        case ColorModelId::Oklab:
            return new OKLABModel();
        case ColorModelId::Oklch:
            return new OKLCHModel();
        case ColorModelId::Okhsv:
            return new OKHSVModel();
        case ColorModelId::Okhsl:
            return new OKHSLModel();
        default:
            return nullptr;
        }
    }

    static ColorModel *fromName(const QString &name)
    {
        for (auto id : AllModels) {
            auto model = fromId(id);
            if (model && model->displayName() == name) {
                return model;
            }
            delete model;
        }
        return nullptr;
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

    static const QVector<ColorModelId> AllModels;
};

#endif
