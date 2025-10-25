#ifndef EXCOLORModel_H
#define EXCOLORModel_H

#include <array>

#include <QVector3D>

#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>

typedef KisSharedPtr<class EXColorModel> ColorModelSP;

enum ColorModelId {
    Gray = 0,
    Srgb = 1,
    Hsv = 2,
    Hsl = 3,
    LinearRgb = 4,
    Xyz = 5,
    Lab = 6,
    Lch = 7,
    Oklab = 8,
    Oklch = 9,
    Okhsv = 10,
    Okhsl = 11,
    Normal = 12,
};

class EXColorModel : public KisShared
{
public:
    virtual ~EXColorModel() = default;

    virtual QVector3D toXyz(const QVector3D &color) const = 0;
    virtual QVector3D fromXyz(const QVector3D &color) const = 0;

    virtual bool isDesaturatable() const
    {
        return false;
    }

    virtual float desaturate(const QVector3D &color) const
    {
        Q_UNUSED(color);
        return 0.0f;
    }

    virtual QVector3D fromDesaturated(float desaturated) const
    {
        Q_UNUSED(desaturated);
        return QVector3D();
    }

    virtual void resolveReference(QVector3D &color, const QVector3D &reference) const
    {
        Q_UNUSED(color);
        Q_UNUSED(reference);
    }

    virtual void makeColorful(QVector3D &color, int channelIndex) const
    {
        Q_UNUSED(color);
        Q_UNUSED(channelIndex);
    }

    virtual int colorfulableChannelIndexBits() const
    {
        return 0;
    }

    virtual int wrappableChannelIndexBits() const
    {
        return 0;
    }

    virtual ColorModelId id() const = 0;
    virtual QString displayName() const = 0;
    virtual quint32 channelCount() const
    {
        return 3;
    }
    virtual std::array<QString, 3> channelNames() const = 0;
    virtual std::array<QVector3D, 2> channelRanges() const = 0;
    virtual bool isSrgbBased() const = 0;

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

    QVector3D transferTo(const EXColorModel *toModel, const QVector3D &color) const;
    QVector3D transferTo(const EXColorModel *toModel, const QVector3D &color, const QVector3D &reference) const;
};

class GrayModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Gray;
    }

    QString displayName() const override
    {
        return "GRAY";
    }

    quint32 channelCount() const override
    {
        return 1;
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"V", "", ""};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(0, 0, 0), QVector3D(100, 0, 0)};
    }

    bool isSrgbBased() const override
    {
        return true;
    }

    static ColorModelSP DesaturateModel;
};

class SRGBModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    bool isDesaturatable() const override
    {
        return true;
    }
    float desaturate(const QVector3D &color) const override;
    QVector3D fromDesaturated(float desaturated) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Srgb;
    }

    QString displayName() const override
    {
        return "SRGB";
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

class HSVModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    bool isDesaturatable() const override
    {
        return true;
    }
    float desaturate(const QVector3D &color) const override;
    QVector3D fromDesaturated(float desaturated) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;
    void makeColorful(QVector3D &color, int channelIndex) const override;

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

    int colorfulableChannelIndexBits() const override
    {
        return 0b001;
    }

    int wrappableChannelIndexBits() const override
    {
        return 0b001;
    }
};

class HSLModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    bool isDesaturatable() const override
    {
        return true;
    }
    float desaturate(const QVector3D &color) const override;
    QVector3D fromDesaturated(float desaturated) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;
    void makeColorful(QVector3D &color, int channelIndex) const override;

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

    int colorfulableChannelIndexBits() const override
    {
        return 0b001;
    }

    int wrappableChannelIndexBits() const override
    {
        return 0b001;
    }
};

class LinearRGBModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    bool isDesaturatable() const override
    {
        return true;
    }
    float desaturate(const QVector3D &color) const override;
    QVector3D fromDesaturated(float desaturated) const override;

    ColorModelId id() const override
    {
        return ColorModelId::LinearRgb;
    }

    QString displayName() const override
    {
        return "LinearRGB";
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

class XYZModel : public EXColorModel
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

class LABModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    bool isDesaturatable() const override
    {
        return true;
    }
    float desaturate(const QVector3D &color) const override;
    QVector3D fromDesaturated(float desaturated) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;

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

class LCHModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;

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

    int wrappableChannelIndexBits() const override
    {
        return 0b100;
    }
};

class OKLABModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    bool isDesaturatable() const override
    {
        return true;
    }
    float desaturate(const QVector3D &color) const override;
    QVector3D fromDesaturated(float desaturated) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;

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

class OKLCHModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;

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

    int wrappableChannelIndexBits() const override
    {
        return 0b100;
    }
};

class OKHSVModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void makeColorful(QVector3D &color, int channelIndex) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;

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

    int colorfulableChannelIndexBits() const override
    {
        return 0b001;
    }

    int wrappableChannelIndexBits() const override
    {
        return 0b001;
    }
};

class OKHSLModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;
    void resolveReference(QVector3D &color, const QVector3D &reference) const override;

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

    int colorfulableChannelIndexBits() const override
    {
        return 0b001;
    }

    int wrappableChannelIndexBits() const override
    {
        return 0b001;
    }
};

class NormalModel : public EXColorModel
{
public:
    QVector3D toXyz(const QVector3D &color) const override;
    QVector3D fromXyz(const QVector3D &color) const override;

    ColorModelId id() const override
    {
        return ColorModelId::Normal;
    }

    QString displayName() const override
    {
        return "Normal";
    }

    quint32 channelCount() const override
    {
        return 2;
    }

    std::array<QString, 3> channelNames() const override
    {
        return {"X", "Y", ""};
    }

    std::array<QVector3D, 2> channelRanges() const override
    {
        return {QVector3D(-100, -100, 0), QVector3D(100, 100, 0)};
    }

    bool isSrgbBased() const override
    {
        return true;
    }

    int wrappableChannelIndexBits() const override
    {
        return 0b011;
    }
};

class ColorModelFactory
{
public:
    static EXColorModel *fromId(ColorModelId id)
    {
        switch (id) {
        case ColorModelId::Gray:
            return new GrayModel();
        case ColorModelId::Srgb:
            return new SRGBModel();
        case ColorModelId::Hsv:
            return new HSVModel();
        case ColorModelId::Hsl:
            return new HSLModel();
        case ColorModelId::LinearRgb:
            return new LinearRGBModel();
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
        case ColorModelId::Normal:
            return new NormalModel();
        default:
            return nullptr;
        }
    }

    static EXColorModel *fromName(const QString &name)
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

    static EXColorModel *fromKoColorSpace(const KoColorSpace *colorSpace)
    {
        auto id = colorSpace->colorModelId();
        if (id == RGBAColorModelID) {
            if (colorSpace->profile()->isLinear()) {
                return new LinearRGBModel();
            } else {
                return new SRGBModel();
            }
            return new LinearRGBModel();
        } else if (id == LABAColorModelID) {
            return new LABModel();
        } else if (id == XYZAColorModelID) {
            return new XYZModel();
        } else if (id == GrayAColorModelID) {
            return new GrayModel();
        } else {
            return nullptr;
        }
    }

    static const QVector<ColorModelId> AllModels;
};

#endif
