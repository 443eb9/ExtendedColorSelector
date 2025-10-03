#include <cmath>
#include <qmath.h>

#include "EXColorModel.h"
#include "ok_color.h"

const QVector<ColorModelId> ColorModelFactory::AllModels = {ColorModelId::Rgb,
                                                            ColorModelId::Hsv,
                                                            ColorModelId::Hsl,
                                                            ColorModelId::Xyz,
                                                            ColorModelId::Lab,
                                                            ColorModelId::Lch,
                                                            ColorModelId::Oklab,
                                                            ColorModelId::Oklch,
                                                            ColorModelId::Okhsv,
                                                            ColorModelId::Okhsl};

QVector3D ColorModel::transferTo(const ColorModel *toModel, const QVector3D &color, const QVector3D *reference) const
{
    if (toModel->id() == id()) {
        return color;
    }

    QVector3D xyz = toXyz(color);
    QVector3D result = toModel->fromXyz(xyz);
    // TODO use reference color
    return result;
}

const float D65_WHITE_XYZ[3]{0.95047, 1.0, 1.08883};
const float CIE_EPSILON = 216.0 / 24389.0;
const float CIE_KAPPA = 24389.0 / 27.0;

QVector3D RGBModel::toXyz(const QVector3D &color) const
{
    float r = color[0], g = color[1], b = color[2];

    float x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
    float y = r * 0.2126729 + g * 0.7151522 + b * 0.072175;
    float z = r * 0.0193339 + g * 0.119192 + b * 0.9503041;

    return QVector3D(x, y, z);
}

QVector3D RGBModel::fromXyz(const QVector3D &color) const
{
    float x = color[0], y = color[1], z = color[2];

    float r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    float g = x * -0.969266 + y * 1.8760108 + z * 0.041556;
    float b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    return QVector3D(r, g, b);
}

QVector3D rgbToHwb(const QVector3D &color)
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
        break;
    };

    return QVector3D(red, green, blue);
}

QVector3D HSVModel::fromXyz(const QVector3D &color) const
{
    QVector3D hwb = rgbToHwb(RGBModel().fromXyz(color));
    float value = 1. - hwb[1];
    float saturation = value != 0. ? 1. - (hwb[1] / value) : 0.;
    return QVector3D(hwb[0], saturation, value);
}

QVector3D HSVModel::toXyz(const QVector3D &color) const
{
    return RGBModel().toXyz(hwbToRgb(QVector3D(color[0], (1. - color[1]) * color[2], 1. - color[2])));
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
    auto hsv = HSVModel().fromXyz(color);
    float lightness = hsv[2] * (1. - hsv[1] / 2.);
    float saturation =
        (lightness == 0.0f || lightness == 1.0f) ? 0.0f : ((color[2] - lightness) / qMin(lightness, 1.0f - lightness));

    return QVector3D(hsv[0], saturation, lightness);
}

QVector3D HSLModel::toXyz(const QVector3D &color) const
{
    float value = color[2] + color[1] * qMin(color[2], 1.0f - color[2]);
    float saturation = value == 0. ? 0. : 2. * (1. - (color[2] / value));

    return HSVModel().toXyz(QVector3D(color[0], saturation, value));
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

QVector3D LABModel::fromXyz(const QVector3D &color) const
{
    float xr = color[0] / D65_WHITE_XYZ[0];
    float yr = color[1] / D65_WHITE_XYZ[1];
    float zr = color[2] / D65_WHITE_XYZ[2];
    float fx = xr > CIE_EPSILON ? cbrtf(xr) : ((CIE_KAPPA * xr + 16.0) / 116.0);
    float fy = yr > CIE_EPSILON ? cbrtf(yr) : ((CIE_KAPPA * yr + 16.0) / 116.0);
    float fz = zr > CIE_EPSILON ? cbrtf(zr) : (CIE_KAPPA * zr + 16.0) / 116.0;
    float l = 1.16 * fy - 0.16;
    float a = 5.00 * (fx - fy);
    float b = 2.00 * (fy - fz);

    return QVector3D(l / 1.5, (a + 1.5) / 3, (b + 1.5) / 3);
}

QVector3D LABModel::toXyz(const QVector3D &color) const
{
    float l = 100. * color[0] * 1.5;
    float a = 100. * (color[1] * 3 - 1.5);
    float b = 100. * (color[2] * 3 - 1.5);

    float fy = (l + 16.0) / 116.0;
    float fx = a / 500.0 + fy;
    float fz = fy - b / 200.0;
    float fx3 = powf(fx, 3.0);
    float xr = fx3 > CIE_EPSILON ? fx3 : ((116.0 * fx - 16.0) / CIE_KAPPA);
    float yr = (l > CIE_EPSILON * CIE_KAPPA) ? powf((l + 16.0) / 116.0, 3.0) : (l / CIE_KAPPA);
    float fz3 = powf(fz, 3.0);
    float zr = fz3 > CIE_EPSILON ? fz3 : ((116.0 * fz - 16.0) / CIE_KAPPA);

    float x = xr * D65_WHITE_XYZ[0];
    float y = yr * D65_WHITE_XYZ[1];
    float z = zr * D65_WHITE_XYZ[2];

    return QVector3D(x, y, z);
}

QVector3D LCHModel::fromXyz(const QVector3D &color) const
{
    auto lab = LABModel().fromXyz(color);
    float a = lab[1] * 2 - 1, b = lab[2] * 2 - 1;
    float c = hypotf(a, b);
    float h = qRadiansToDegrees(atan2f(b, a));
    if (h < 0.0) {
        h += 360.0;
    }
    float chroma = qBound(0.0f, c, 1.5f);

    return QVector3D(lab[0], c / 1.5, h / 360);
}

QVector3D LCHModel::toXyz(const QVector3D &color) const
{
    float sin, cos;
    sincosf(qDegreesToRadians(color[2] * 360), &sin, &cos);
    float a = color[1] * cos;
    float b = color[1] * sin;

    return LABModel().toXyz(QVector3D(color[0], a * 0.5 + 0.5, b * 0.5 + 0.5));
}

// https:#bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
QVector3D OKLABModel::fromXyz(const QVector3D &color) const
{
    float x = color[0], y = color[1], z = color[2];

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

    return QVector3D(x, y, z);
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
    float sin, cos;
    sincosf(qDegreesToRadians(color[2] * 360), &sin, &cos);
    float a = color[1] * cos;
    float b = color[1] * sin;

    return OKLABModel().toXyz(QVector3D(color[0], a * 0.5 + 0.5, b * 0.5 + 0.5));
}

QVector3D OKHSVModel::fromXyz(const QVector3D &color) const
{
    auto rgb = RGBModel().fromXyz(color);
    auto okhsv = ok_color::linear_rgb_to_okhsv(ok_color::RGB{rgb[0], rgb[1], rgb[2]});
    return QVector3D(okhsv.h, okhsv.s, okhsv.v);
}

QVector3D OKHSVModel::toXyz(const QVector3D &color) const
{
    auto rgb = ok_color::okhsv_to_linear_rgb(ok_color::HSV{color[0], color[1], color[2]});
    auto xyz = RGBModel().toXyz(QVector3D(rgb.r, rgb.g, rgb.b));
    return QVector3D(xyz[0], xyz[1], xyz[2]);
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
    auto rgb = RGBModel().fromXyz(color);
    auto okhsl = ok_color::linear_rgb_to_okhsl(ok_color::RGB{rgb[0], rgb[1], rgb[2]});
    return QVector3D(okhsl.h, okhsl.s, okhsl.l);
}

QVector3D OKHSLModel::toXyz(const QVector3D &color) const
{
    auto rgb = ok_color::okhsl_to_linear_rgb(ok_color::HSL{color[0], color[1], color[2]});
    auto xyz = RGBModel().toXyz(QVector3D(rgb.r, rgb.g, rgb.b));
    return QVector3D(xyz[0], xyz[1], xyz[2]);
}
