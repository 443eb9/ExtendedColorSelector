#include <array>
#include <cmath>
#include <memory>
#include <vector>

#include <QDebug>
#include <QVector2D>
#include <qglobal.h>
#include <qmath.h>
#include <qvector3d.h>

#include <Eigen/LU>

#include <KoChannelInfo.h>
#include <KoColor.h>
#include <KoColorConversionTransformation.h>

#include "EXColorModel.h"
#include "ok_color.h"

const float EPSILON = 1e-4f;

const QVector<ColorModelId> ColorModelFactory::AllModels = {ColorModelId::Gray,
                                                            ColorModelId::Rgb,
                                                            ColorModelId::Hsv,
                                                            ColorModelId::Hsl,
                                                            ColorModelId::Xyz,
                                                            ColorModelId::Lab,
                                                            ColorModelId::Lch,
                                                            ColorModelId::Oklab,
                                                            ColorModelId::Oklch,
                                                            ColorModelId::Okhsv,
                                                            ColorModelId::Okhsl,
                                                            ColorModelId::Normal};

namespace
{
std::array<int, 4> logicalToMemoryPositions(const KoColorSpace *colorSpace)
{
    std::array<int, 4> positions{-1, -1, -1, -1};
    const QList<KoChannelInfo *> channels = colorSpace->channels();
    for (int memoryPosition = 0; memoryPosition < channels.size(); ++memoryPosition) {
        const int logicalPosition = channels[memoryPosition]->displayPosition();
        if (logicalPosition >= 0 && logicalPosition < static_cast<int>(positions.size())) {
            positions[logicalPosition] = memoryPosition;
        }
    }
    return positions;
}

class ThreadColorConverter
{
public:
    ThreadColorConverter(const KoColorSpace *source, const KoColorSpace *destination)
        : m_sourceSpace(source)
        , m_destinationSpace(destination)
        , m_transform(source->createColorConverter(destination,
                                                   KoColorConversionTransformation::IntentRelativeColorimetric,
                                                   KoColorConversionTransformation::ConversionFlags()))
        , m_sourceColor(source)
        , m_destinationColor(destination)
        , m_sourceChannels(source->channelCount())
        , m_destinationChannels(destination->channelCount())
        , m_sourcePositions(logicalToMemoryPositions(source))
        , m_destinationPositions(logicalToMemoryPositions(destination))
    {
    }

    bool matches(const KoColorSpace *source, const KoColorSpace *destination) const
    {
        return source == m_sourceSpace && destination == m_destinationSpace;
    }

    bool convert(const QVector3D &source, QVector3D &destination)
    {
        if (!m_transform) {
            return false;
        }

        m_sourceChannels.fill(0.0f);
        for (int i = 0; i < 3; ++i) {
            if (m_sourcePositions[i] >= 0) {
                m_sourceChannels[m_sourcePositions[i]] = source[i];
            }
        }
        if (m_sourcePositions[3] >= 0) {
            m_sourceChannels[m_sourcePositions[3]] = 1.0f;
        }

        m_sourceSpace->fromNormalisedChannelsValue(m_sourceColor.data(), m_sourceChannels);
        m_transform->transform(m_sourceColor.data(), m_destinationColor.data(), 1);
        m_destinationSpace->normalisedChannelsValue(m_destinationColor.data(), m_destinationChannels);

        destination = QVector3D();
        for (int i = 0; i < 3; ++i) {
            if (m_destinationPositions[i] >= 0) {
                destination[i] = m_destinationChannels[m_destinationPositions[i]];
            }
        }
        return true;
    }

private:
    const KoColorSpace *m_sourceSpace;
    const KoColorSpace *m_destinationSpace;
    std::unique_ptr<KoColorConversionTransformation> m_transform;
    KoColor m_sourceColor;
    KoColor m_destinationColor;
    QVector<float> m_sourceChannels;
    QVector<float> m_destinationChannels;
    std::array<int, 4> m_sourcePositions;
    std::array<int, 4> m_destinationPositions;
};

ThreadColorConverter *threadColorConverter(const KoColorSpace *source, const KoColorSpace *destination)
{
    static thread_local std::vector<std::unique_ptr<ThreadColorConverter>> converters;
    for (const auto &converter : converters) {
        if (converter->matches(source, destination)) {
            return converter.get();
        }
    }

    converters.push_back(std::make_unique<ThreadColorConverter>(source, destination));
    return converters.back().get();
}
} // namespace

QVector3D EXColorModel::transferTo(const EXColorModel *toModel, const QVector3D &color) const
{
    return toModel->fromXyz(toXyz(color));
}

QVector3D
EXColorModel::transferTo(const EXColorModel *toModel, const QVector3D &color, const QVector3D &reference) const
{
    auto result = toModel->fromXyz(toXyz(color));
    toModel->resolveReference(result, reference);
    return result;
}

QVector3D EXColorModel::transferToWithGamutWarning(const EXColorModel *toModel,
                                                   const QVector3D &color,
                                                   const QVector3D &outOfGamutColor) const
{
    return toModel->fromXyz(toXyz(color), outOfGamutColor);
}

const QVector3D D50_WHITE_XYZ(0.96422f, 1.0f, 0.82521f);
const QVector3D D65_WHITE_XYZ(0.95047f, 1.0f, 1.08883f);
const float CIE_EPSILON = 216.0 / 24389.0;
const float CIE_KAPPA = 24389.0 / 27.0;

static const Eigen::Matrix3f SRGB_TO_XYZ_D65 = (Eigen::Matrix3f() << 0.4124564f,
                                                0.3575761f,
                                                0.1804375f,
                                                0.2126729f,
                                                0.7151522f,
                                                0.0721750f,
                                                0.0193339f,
                                                0.1191920f,
                                                0.9503041f)
                                                   .finished();
static const Eigen::Matrix3f XYZ_TO_SRGB_D65 = SRGB_TO_XYZ_D65.inverse();

static QVector3D multiplyMatrix(const Eigen::Matrix3f &matrix, const QVector3D &vector)
{
    const Eigen::Vector3f result = matrix * Eigen::Vector3f(vector.x(), vector.y(), vector.z());
    return QVector3D(result.x(), result.y(), result.z());
}

static Eigen::Matrix3f createBradfordAdaptation(const QVector3D &sourceWhite, const QVector3D &destinationWhite)
{
    static const Eigen::Matrix3f bradford =
        (Eigen::Matrix3f() << 0.8951f, 0.2664f, -0.1614f, -0.7502f, 1.7135f, 0.0367f, 0.0389f, -0.0685f, 1.0296f)
            .finished();

    const Eigen::Vector3f sourceCone = bradford * Eigen::Vector3f(sourceWhite.x(), sourceWhite.y(), sourceWhite.z());
    const Eigen::Vector3f destinationCone =
        bradford * Eigen::Vector3f(destinationWhite.x(), destinationWhite.y(), destinationWhite.z());
    Eigen::Vector3f scale;

    for (int i = 0; i < 3; i++) {
        scale[i] = qAbs(sourceCone[i]) > 1e-8f ? destinationCone[i] / sourceCone[i] : 1.0f;
    }

    return bradford.inverse() * scale.asDiagonal() * bradford;
}

static QVector3D adaptWhitePointBradford(const QVector3D &color, const Eigen::Matrix3f &adaptation)
{
    return multiplyMatrix(adaptation, color);
}

static const Eigen::Matrix3f D50_TO_D65_BRADFORD = createBradfordAdaptation(D50_WHITE_XYZ, D65_WHITE_XYZ);
static const Eigen::Matrix3f D65_TO_D50_BRADFORD = createBradfordAdaptation(D65_WHITE_XYZ, D50_WHITE_XYZ);

static QVector3D linearRgbToXyz(const QVector3D &color)
{
    return multiplyMatrix(SRGB_TO_XYZ_D65, color);
}

static QVector3D xyzToLinearRgb(const QVector3D &color)
{
    return multiplyMatrix(XYZ_TO_SRGB_D65, color);
}

ColorModelSP GrayModel::DesaturateModel = new OKLABModel();

QVector3D GrayModel::toXyz(const QVector3D &color) const
{
    auto c = DesaturateModel->fromDesaturated(color[0]);
    return DesaturateModel->toXyz(c);
}

QVector3D GrayModel::fromXyz(const QVector3D &color) const
{
    QVector3D c = DesaturateModel->fromXyz(color);
    return QVector3D(DesaturateModel->desaturate(c), 0.0f, 0.0f);
}

void RGBModel::setProfile(const KoColorProfile *profile)
{
    KoColorSpaceRegistry *registry = KoColorSpaceRegistry::instance();
    m_profile = profile ? profile : registry->p709SRGBProfile();
    m_rgbColorSpace = registry->colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(), m_profile);
    m_xyzColorSpace = registry->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id());

    if (!m_rgbColorSpace || !m_xyzColorSpace) {
        qWarning() << "ExtendedColorSelector: failed to create managed RGB/XYZ color spaces for profile"
                   << (m_profile ? m_profile->name() : QString());
    }
}

QVector3D RGBModel::fromXyz(const QVector3D &color) const
{
    QVector3D result;
    if (!m_rgbColorSpace || !m_xyzColorSpace
        || !threadColorConverter(m_xyzColorSpace, m_rgbColorSpace)->convert(color, result)) {
        return QVector3D();
    }
    return result;
}

QVector3D RGBModel::fromXyz(const QVector3D &color, const QVector3D &outOfGamutFallback) const
{
    QVector3D result;
    QVector3D roundtripXyz;
    if (!m_rgbColorSpace || !m_xyzColorSpace
        || !threadColorConverter(m_xyzColorSpace, m_rgbColorSpace)->convert(color, result)
        || !threadColorConverter(m_rgbColorSpace, m_xyzColorSpace)->convert(result, roundtripXyz)) {
        return QVector3D();
    }

    LABModel labModel;
    const QVector3D sourceLab = labModel.unnormalize(labModel.fromXyz(color));
    const QVector3D roundtripLab = labModel.unnormalize(labModel.fromXyz(roundtripXyz));
    const float deltaE = (sourceLab - roundtripLab).length();

    return !qIsFinite(deltaE) || deltaE > 0.1f ? outOfGamutFallback : result;
}

QVector3D RGBModel::toXyz(const QVector3D &color) const
{
    QVector3D result;
    if (!m_rgbColorSpace || !m_xyzColorSpace
        || !threadColorConverter(m_rgbColorSpace, m_xyzColorSpace)->convert(color, result)) {
        return QVector3D();
    }
    return result;
}

float RGBModel::desaturate(const QVector3D &color) const
{
    return 0.2126f * color[0] + 0.7152f * color[1] + 0.0722f * color[2];
}

QVector3D RGBModel::fromDesaturated(float desaturated) const
{
    return QVector3D(desaturated, desaturated, desaturated);
}

QVector3D srgbToHwb(const QVector3D &color)
{
    float red = color[0], green = color[1], blue = color[2];
    float x_max = qMax((float)0, qMax(red, qMax(green, blue)));
    float x_min = qMin((float)1, qMin(red, qMin(green, blue)));

    float chroma = x_max - x_min;

    float hue;
    if (chroma == 0.0) {
        hue = 0.0;
    } else if (red == x_max) {
        hue = 60.0 * (green - blue) / chroma;
    } else if (green == x_max) {
        hue = 60.0 * (2.0 + (blue - red) / chroma);
    } else {
        hue = 60.0 * (4.0 + (red - green) / chroma);
    };

    hue = hue < 0.0 ? 360.0 + hue : hue;

    float whiteness = x_min;
    float blackness = 1.0 - x_max;
    return QVector3D(hue / 360.0, whiteness, blackness);
}

QVector3D hwbToRgb(const QVector3D &color)
{
    float w = color[1];
    float v = 1. - color[2];

    float h = fmodf(color[0] * 360., 360.) / 60.;
    float i = floorf(h);
    float f = h - i;

    int ii = i;

    float ff = ii % 2 == 0 ? f : 1. - f;

    float n = w + ff * (v - w);

    float red, green, blue;

    switch (ii) {
    case 0:
        red = v, green = n, blue = w;
        break;
    case 1:
        red = n, green = v, blue = w;
        break;
    case 2:
        red = w, green = v, blue = n;
        break;
    case 3:
        red = w, green = n, blue = v;
        break;
    case 4:
        red = n, green = w, blue = v;
        break;
    case 5:
        red = v, green = w, blue = n;
        break;
    default:
        red = v, green = n, blue = w;
        break;
    };

    return QVector3D(red, green, blue);
}

QVector3D HSVModel::fromXyz(const QVector3D &color) const
{
    QVector3D hwb = srgbToHwb(m_rgbModel.fromXyz(color));
    float value = 1. - hwb[2];
    float saturation = value != 0. ? 1. - (hwb[1] / value) : 0.;
    return QVector3D(hwb[0], saturation, value);
}

QVector3D HSVModel::toXyz(const QVector3D &color) const
{
    return m_rgbModel.toXyz(hwbToRgb(QVector3D(color[0], (1. - color[1]) * color[2], 1. - color[2])));
}

float HSVModel::desaturate(const QVector3D &color) const
{
    return color[2];
}

QVector3D HSVModel::fromDesaturated(float desaturated) const
{
    return QVector3D(0.0f, 0.0f, desaturated);
}

void HSVModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[1] < EPSILON) {
        color[0] = reference[0];
    }

    if (color[2] < EPSILON) {
        color[0] = reference[0];
        color[1] = reference[1];
    }
}

void HSVModel::makeColorful(QVector3D &color, int channelIndex) const
{
    if (channelIndex == 0) {
        color[1] = 1.0;
        color[2] = 1.0;
    }
}

QVector3D HSLModel::fromXyz(const QVector3D &color) const
{
    auto hsv = m_hsvModel.fromXyz(color);
    float saturation = hsv[1], value = hsv[2];
    float lightness = value * (1. - saturation / 2.);
    saturation = (lightness == 0. || lightness == 1.) ? 0. : (value - lightness) / qMin(lightness, 1.f - lightness);

    return QVector3D(hsv[0], saturation, lightness);
}

QVector3D HSLModel::toXyz(const QVector3D &color) const
{
    float saturation = color[1], lightness = color[2];
    float value = lightness + saturation * qMin(lightness, 1.f - lightness);
    saturation = value == 0. ? 0. : 2. * (1. - (lightness / value));

    return m_hsvModel.toXyz(QVector3D(color[0], saturation, value));
}

float HSLModel::desaturate(const QVector3D &color) const
{
    return color[2];
}

QVector3D HSLModel::fromDesaturated(float desaturated) const
{
    return QVector3D(0.0f, 0.0f, desaturated);
}

void HSLModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[1] < EPSILON) {
        color[0] = reference[0];
    }

    if (color[2] < EPSILON || color[2] > 1.0f - EPSILON) {
        color[0] = reference[0];
        color[1] = reference[1];
    }
}

void HSLModel::makeColorful(QVector3D &color, int channelIndex) const
{
    if (channelIndex == 0) {
        color[1] = 1.0;
        color[2] = 0.5;
    }
}

QVector3D XYZModel::fromXyz(const QVector3D &color) const
{
    return color;
}

QVector3D XYZModel::toXyz(const QVector3D &color) const
{
    return color;
}

static float normalizeLabAB(float value)
{
    return value <= 0.0f ? (value + 128.0f) / 256.0f : 0.5f + value / 254.0f;
}

static float unnormalizeLabAB(float normalized)
{
    return normalized <= 0.5f ? normalized * 256.0f - 128.0f : (normalized - 0.5f) * 254.0f;
}

QVector3D LABModel::fromXyz(const QVector3D &color) const
{
    const float xr = color[0] / D50_WHITE_XYZ[0];
    const float yr = color[1] / D50_WHITE_XYZ[1];
    const float zr = color[2] / D50_WHITE_XYZ[2];
    const float fx = xr > CIE_EPSILON ? cbrtf(xr) : ((CIE_KAPPA * xr + 16.0f) / 116.0f);
    const float fy = yr > CIE_EPSILON ? cbrtf(yr) : ((CIE_KAPPA * yr + 16.0f) / 116.0f);
    const float fz = zr > CIE_EPSILON ? cbrtf(zr) : ((CIE_KAPPA * zr + 16.0f) / 116.0f);
    const float l = 116.0f * fy - 16.0f;
    const float a = 500.0f * (fx - fy);
    const float b = 200.0f * (fy - fz);

    return normalize(QVector3D(l, a, b));
}

QVector3D LABModel::toXyz(const QVector3D &color) const
{
    const QVector3D lab = unnormalize(color);
    const float l = lab[0];
    const float a = lab[1];
    const float b = lab[2];

    const float fy = (l + 16.0f) / 116.0f;
    const float fx = a / 500.0f + fy;
    const float fz = fy - b / 200.0f;
    const float fx3 = powf(fx, 3.0f);
    const float xr = fx3 > CIE_EPSILON ? fx3 : ((116.0f * fx - 16.0f) / CIE_KAPPA);
    const float yr = (l > CIE_EPSILON * CIE_KAPPA) ? powf(fy, 3.0f) : (l / CIE_KAPPA);
    const float fz3 = powf(fz, 3.0f);
    const float zr = fz3 > CIE_EPSILON ? fz3 : ((116.0f * fz - 16.0f) / CIE_KAPPA);

    return QVector3D(xr * D50_WHITE_XYZ[0], yr * D50_WHITE_XYZ[1], zr * D50_WHITE_XYZ[2]);
}

float LABModel::desaturate(const QVector3D &color) const
{
    return color[0];
}

QVector3D LABModel::fromDesaturated(float desaturated) const
{
    return QVector3D(desaturated, 0.5f, 0.5f);
}

void LABModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[0] < EPSILON || color[0] > 1.0f - EPSILON) {
        color[1] = reference[1];
        color[2] = reference[2];
    }
}

QVector3D LABModel::unnormalize(const QVector3D &normalized) const
{
    return QVector3D(normalized[0] * 100.0f, unnormalizeLabAB(normalized[1]), unnormalizeLabAB(normalized[2]));
}

QVector3D LABModel::normalize(const QVector3D &value) const
{
    return QVector3D(value[0] / 100.0f, normalizeLabAB(value[1]), normalizeLabAB(value[2]));
}

QVector3D LCHModel::fromXyz(const QVector3D &color) const
{
    LABModel labModel;
    const QVector3D lab = labModel.unnormalize(labModel.fromXyz(color));
    const float c = hypotf(lab[1], lab[2]);
    float h = qRadiansToDegrees(atan2f(lab[2], lab[1]));
    if (h < 0.0f) {
        h += 360.0f;
    }

    return normalize(QVector3D(lab[0], c, h));
}

QVector3D LCHModel::toXyz(const QVector3D &color) const
{
    const QVector3D lch = unnormalize(color);
    const float h = qDegreesToRadians(lch[2]);
    const float a = lch[1] * cosf(h);
    const float b = lch[1] * sinf(h);

    LABModel labModel;
    return labModel.toXyz(labModel.normalize(QVector3D(lch[0], a, b)));
}

void LCHModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[0] < EPSILON || color[0] > 1.0f - EPSILON) {
        color[1] = reference[1];
        color[2] = reference[2];
    }

    if (color[1] < EPSILON) {
        color[2] = reference[2];
    }
}

// https:#bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
QVector3D OKLABModel::fromXyz(const QVector3D &color) const
{
    const QVector3D d65Color = adaptWhitePointBradford(color, D50_TO_D65_BRADFORD);
    float x = d65Color[0], y = d65Color[1], z = d65Color[2];

    float l_ = 0.8189330101 * x + 0.3618667424 * y - 0.1288597137 * z;
    float m_ = 0.0329845436 * x + 0.9293118715 * y + 0.0361456387 * z;
    float s_ = 0.0482003018 * x + 0.2643662691 * y + 0.6338517070 * z;

    l_ = cbrtf(l_);
    m_ = cbrtf(m_);
    s_ = cbrtf(s_);

    float l = 0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_;
    float a = 1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_;
    float b = 0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_;

    return QVector3D(l, a * 0.5 + 0.5, b * 0.5 + 0.5);
}

// https:#bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
// Inverse matrices are computed from the matrix in the post
QVector3D OKLABModel::toXyz(const QVector3D &color) const
{
    float l = color[0], a = color[1] * 2 - 1, b = color[2] * 2 - 1;

    float l_ = 0.9999999984 * l + 0.3963377921 * a + 0.2158037580 * b;
    float m_ = 1.0000000088 * l - 0.10556134232 * a - 0.0638541747 * b;
    float s_ = 1.0000000546 * l - 0.08948418209 * a - 1.2914855378 * b;

    l_ = powf(l_, 3);
    m_ = powf(m_, 3);
    s_ = powf(s_, 3);

    float x = +1.2270138511 * l_ - 0.5577999806 * m_ + 0.2812561489 * s_;
    float y = -0.0405801784 * l_ + 1.1122568696 * m_ - 0.0716766786 * s_;
    float z = -0.0763812845 * l_ - 0.4214819784 * m_ + 1.5861632204 * s_;

    return adaptWhitePointBradford(QVector3D(x, y, z), D65_TO_D50_BRADFORD);
}

float OKLABModel::desaturate(const QVector3D &color) const
{
    return color[0];
}

QVector3D OKLABModel::fromDesaturated(float desaturated) const
{
    return QVector3D(desaturated, 0.5f, 0.5f);
}

void OKLABModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[0] < EPSILON || color[0] > 1.0f - EPSILON) {
        color[1] = reference[1];
        color[2] = reference[2];
    }
}

QVector3D OKLCHModel::fromXyz(const QVector3D &color) const
{
    auto oklab = OKLABModel().fromXyz(color);
    float a = oklab[1] * 2 - 1, b = oklab[2] * 2 - 1;

    float chroma = hypotf(a, b);
    float hue = qRadiansToDegrees(atan2f(b, a));
    if (hue < 0) {
        hue += 360;
    }

    return QVector3D(oklab[0], chroma, hue / 360);
}

QVector3D OKLCHModel::toXyz(const QVector3D &color) const
{
    float h = qDegreesToRadians(color[2] * 360);
    float a = color[1] * cosf(h);
    float b = color[1] * sinf(h);

    return OKLABModel().toXyz(QVector3D(color[0], a * 0.5 + 0.5, b * 0.5 + 0.5));
}

void OKLCHModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[0] < EPSILON || color[0] > 1.0f - EPSILON) {
        color[1] = reference[1];
        color[2] = reference[2];
    }

    if (color[1] < EPSILON) {
        color[2] = reference[2];
    }
}

QVector3D OKHSVModel::fromXyz(const QVector3D &color) const
{
    const QVector3D d65Color = adaptWhitePointBradford(color, D50_TO_D65_BRADFORD);
    auto rgb = xyzToLinearRgb(d65Color);
    auto okhsv = ok_color::linear_rgb_to_okhsv(ok_color::RGB{rgb[0], rgb[1], rgb[2]});
    // Avoid singularity
    okhsv.s = qBound(0.0f, okhsv.s, 1.0f - 1e-3f);
    return QVector3D(okhsv.h, okhsv.s, okhsv.v);
}

QVector3D OKHSVModel::toXyz(const QVector3D &color) const
{
    auto rgb = ok_color::okhsv_to_linear_rgb(ok_color::HSV{color[0], qBound(0.0f, color[1], 1.0f - 1e-3f), color[2]});
    auto xyz = linearRgbToXyz(QVector3D(rgb.r, rgb.g, rgb.b));
    return adaptWhitePointBradford(xyz, D65_TO_D50_BRADFORD);
}

void OKHSVModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[1] < EPSILON) {
        color[0] = reference[0];
    }

    if (color[2] < EPSILON) {
        color[0] = reference[0];
        color[1] = reference[1];
    }
}

void OKHSVModel::makeColorful(QVector3D &color, int channelIndex) const
{
    if (channelIndex == 0) {
        color[1] = 1.0;
        color[2] = 1.0;
    }
}

QVector3D OKHSLModel::fromXyz(const QVector3D &color) const
{
    const QVector3D d65Color = adaptWhitePointBradford(color, D50_TO_D65_BRADFORD);
    auto rgb = xyzToLinearRgb(d65Color);
    auto okhsl = ok_color::linear_rgb_to_okhsl(ok_color::RGB{rgb[0], rgb[1], rgb[2]});
    return QVector3D(okhsl.h, okhsl.s, okhsl.l);
}

QVector3D OKHSLModel::toXyz(const QVector3D &color) const
{
    auto rgb = ok_color::okhsl_to_linear_rgb(ok_color::HSL{color[0], color[1], color[2]});
    auto xyz = linearRgbToXyz(QVector3D(rgb.r, rgb.g, rgb.b));
    return adaptWhitePointBradford(xyz, D65_TO_D50_BRADFORD);
}

void OKHSLModel::resolveReference(QVector3D &color, const QVector3D &reference) const
{
    if (color[1] < EPSILON) {
        color[0] = reference[0];
    }

    if (color[2] < EPSILON || color[2] > 1.0f - EPSILON) {
        color[0] = reference[0];
        color[1] = reference[1];
    }
}

QVector3D NormalModel::toXyz(const QVector3D &color) const
{
    auto normalXy = color.toVector2D() * 2.0f - QVector2D(1.0f, 1.0f);
    auto lenSq = normalXy.lengthSquared();
    if (lenSq > 1.0f) {
        normalXy /= sqrtf(lenSq);
        lenSq = 1.0f;
    }
    float z = sqrtf(qBound(0.0f, 1.0f - lenSq, 1.0f));
    auto normal = QVector3D(normalXy.x(), normalXy.y(), z);
    const QVector3D d65Color = linearRgbToXyz(normal * 0.5f + QVector3D(0.5f, 0.5f, 0.5f));
    return adaptWhitePointBradford(d65Color, D65_TO_D50_BRADFORD);
}

QVector3D NormalModel::fromXyz(const QVector3D &color) const
{
    const QVector3D d65Color = adaptWhitePointBradford(color, D50_TO_D65_BRADFORD);
    auto rgb = xyzToLinearRgb(d65Color);
    auto normal = rgb * 2.0f - QVector3D(1.0f, 1.0f, 1.0f);
    auto normalXy = normal.toVector2D();
    normalXy = (normalXy + QVector2D(1.0f, 1.0f)) * 0.5f;
    return normalXy.toVector3D();
}
