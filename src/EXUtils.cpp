#include <qmath.h>

#include "EXColorModel.h"
#include "EXUtils.h"

namespace ExtendedUtils
{
QImage generateGradient(int width,
                        int height,
                        const KoColorSpace *colorSpace,
                        const KoColorDisplayRendererInterface *dri,
                        std::function<void(float, float, QVector<float> &)> pixelGet)
{
    // const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const qreal deviceDivider = 1.0;
    const int deviceWidth = qCeil(width * deviceDivider);
    const int deviceHeight = qCeil(height * deviceDivider);
    const qsizetype pixelSize = colorSpace->pixelSize();
    quint32 imageSize = deviceWidth * deviceHeight * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();
    RGBModel rgbConverter;

    QVector<float> channels(4, 1);
    KoColor color(colorSpace);
    for (int y = 0; y < deviceHeight; y++) {
        for (int x = 0; x < deviceWidth; x++) {
            pixelGet((float)x / (width - 1), (float)y / (height - 1), channels);

            colorSpace->fromNormalisedChannelsValue(color.data(), channels);
            memcpy(dataPtr, color.data(), pixelSize);
            dataPtr += pixelSize;
        }
    }

    QImage image = dri->toQImage(colorSpace, raw.data(), QSize(deviceWidth, deviceHeight), false);
    // image.setDevicePixelRatio(devicePixelRatioF());

    return image;
}

void sanitizeOutOfGamutColor(QVector3D &color, const QVector3D &outOfGamutColor)
{
    const float EPSILON = 1e-5;
    if (color[0] < -EPSILON || color[0] > 1 + EPSILON || color[1] < -EPSILON || color[1] > 1 + EPSILON
        || color[2] < -EPSILON || color[2] > 1 + EPSILON) {
        color = outOfGamutColor;
    }
}

void saturateColor(QVector3D &color)
{
    color[0] = qBound(0.0f, color[0], 1.0f);
    color[1] = qBound(0.0f, color[1], 1.0f);
    color[2] = qBound(0.0f, color[2], 1.0f);
}
} // namespace ExtendedUtils
