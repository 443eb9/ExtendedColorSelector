#ifndef EXTENDEDUTILS_H
#define EXTENDEDUTILS_H

#include <QImage>
#include <QVector3D>
#include <QVector>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>

namespace ExtendedUtils
{
QImage generateGradient(int width,
                        int height,
                        bool useParallel,
                        const KoColorSpace *colorSpace,
                        const KoColorDisplayRendererInterface *dri,
                        std::function<void(float, float, QVector<float> &)> pixelGet);

void sanitizeOutOfGamutColor(QVector3D &color, const QVector3D &outOfGamutColor);
void saturateColor(QVector3D &color);
float getRingValue(QPointF widgetCoordCentered, float rotationOffset);
QString colorToString(QVector3D color);
QVector3D stringToColor(const QString &str);
template<typename T>
QString vectorToString(const QVector<T> &vec, std::function<QString(const T&)> toStringFunc) {
    QStringList parts;
    for (const T &v : vec) {
        parts.append(toStringFunc(v));
    }
    return parts.join(',');
}
template<typename T>
QVector<T> stringToVector(const QString &str, std::function<T(const QString&)> fromStringFunc) {
    QStringList parts = str.split(',', Qt::SkipEmptyParts);
    QVector<T> vec;
    for (const QString &part : parts) {
        vec.append(fromStringFunc(part.trimmed()));
    }
    return vec;
}
} // namespace ExtendedUtils

#endif // EXTENDEDUTILS_H
