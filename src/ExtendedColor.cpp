#include "ExtendedColor.h"

QVector3D RgbConverter::toXyz(const QVector3D &color)
{
    float r = color.x();
    float g = color.y();
    float b = color.z();

    float x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
    float y = r * 0.2126729 + g * 0.7151522 + b * 0.072175;
    float z = r * 0.0193339 + g * 0.119192 + b * 0.9503041;

    return QVector3D(x, y, z);
}

QVector3D RgbConverter::fromXyz(const QVector3D &color)
{
    float x = color.x();
    float y = color.y();
    float z = color.z();

    float r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    float g = x * -0.969266 + y * 1.8760108 + z * 0.041556;
    float b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    return QVector3D(r, g, b);
}
