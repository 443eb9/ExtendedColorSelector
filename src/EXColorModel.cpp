#include <cmath>
#include <qmath.h>

#include "EXColorModel.h"

const ColorModelId ColorModelFactory::AllModels[] = {ColorModelId::Rgb, ColorModelId::Hsv};

QVector3D rgbToXyz(const QVector3D &color)
{
    float r = color[0], g = color[1], b = color[2];

    float x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
    float y = r * 0.2126729 + g * 0.7151522 + b * 0.072175;
    float z = r * 0.0193339 + g * 0.119192 + b * 0.9503041;

    return QVector3D(x, y, z);
}

QVector3D xyzToRgb(const QVector3D &color)
{
    float x = color[0], y = color[1], z = color[2];

    float r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    float g = x * -0.969266 + y * 1.8760108 + z * 0.041556;
    float b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    return QVector3D(r, g, b);
}

QVector3D RGBModel::toXyz(const QVector3D &color) const
{
    return rgbToXyz(color);
}

QVector3D RGBModel::fromXyz(const QVector3D &color) const
{
    return xyzToRgb(color);
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
    QVector3D hwb = rgbToHwb(xyzToRgb(color));
    float value = 1. - hwb[1];
    float saturation = value != 0. ? 1. - (hwb[1] / value) : 0.;
    return QVector3D(hwb[0], saturation, value);
}

QVector3D HSVModel::toXyz(const QVector3D &color) const
{
    return rgbToXyz(hwbToRgb(QVector3D(color[0], (1. - color[1]) * color[2], 1. - color[2])));
}
