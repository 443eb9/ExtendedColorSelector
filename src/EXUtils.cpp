#include <QtConcurrent>
#include <qmath.h>

#include "EXColorModel.h"
#include "EXUtils.h"

namespace ExtendedUtils
{
QImage generateGradient(int width,
                        int height,
                        bool useParallel,
                        const KoColorSpace *colorSpace,
                        const KoColorDisplayRendererInterface *dri,
                        std::function<void(float, float, QVector<float> &)> pixelGet)
{
    const int deviceWidth = qCeil(width);
    const int deviceHeight = qCeil(height);
    const qsizetype pixelSize = colorSpace->pixelSize();
    quint32 imageSize = deviceWidth * deviceHeight * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();

    auto processRow = [&](int y) {
        QVector<float> channels(4, 1);
        KoColor color(colorSpace);
        quint8 *rowPtr = dataPtr + y * deviceWidth * pixelSize;
        
        for (int x = 0; x < deviceWidth; x++) {
            pixelGet((float)x / (width - 1), (float)y / (height - 1), channels);
            colorSpace->fromNormalisedChannelsValue(color.data(), channels);
            memcpy(rowPtr, color.data(), pixelSize);
            rowPtr += pixelSize;
        }
    };
    
    if (useParallel) {
        QVector<int> rows(deviceHeight);
        std::iota(rows.begin(), rows.end(), 0);
        QtConcurrent::blockingMap(rows, processRow);
    } else {
        for (int y = 0; y < deviceHeight; y++) {
            processRow(y);
        }
    }

    return dri->toQImage(colorSpace, raw.data(), QSize(deviceWidth, deviceHeight), false);
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

QString colorToString(QVector3D color)
{
    return QString::number(color[0], 'f', 4) + "," + QString::number(color[1], 'f', 4) + ","
        + QString::number(color[2], 'f', 4);
}

QVector3D stringToColor(const QString &str)
{
    QStringList parts = str.split(',');
    if (parts.size() != 3) {
        return QVector3D(0.0f, 0.0f, 0.0f);
    }
    bool ok1, ok2, ok3;
    float r = parts[0].toFloat(&ok1);
    float g = parts[1].toFloat(&ok2);
    float b = parts[2].toFloat(&ok3);
    if (!ok1 || !ok2 || !ok3) {
        return QVector3D(0.0f, 0.0f, 0.0f);
    }
    return QVector3D(r, g, b);
}
} // namespace ExtendedUtils
