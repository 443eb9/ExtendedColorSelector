#ifndef EXTENDEDUTILS_H
#define EXTENDEDUTILS_H

#include <QImage>
#include <QVector3D>
#include <QVector>
#include <functional>

#include "KisGLImageF16.h"
#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_display_color_converter.h>

#include "EXColorModel.h"
#include "EXEditable.h"
#include "EXKoColorConverter.h"

namespace ExtendedUtils
{
void loadImageIntoEditableWidget(EXEditableImage *editable,
                                 int width,
                                 int height,
                                 bool useParallel,
                                 const KoColorSpace *generationColorSpace,
                                 const EXColorConverterSP colorConverter,
                                 const KisDisplayColorConverter *displayConverter,
                                 std::function<QVector4D(float, float)> pixelGet);

QImage generateGradient(int width,
                        int height,
                        bool useParallel,
                        const EXColorConverterSP colorConverter,
                        const KoColorDisplayRendererInterface *dri,
                        std::function<QVector4D(float, float)> pixelGet);

KisGLImageF16 generateGLGradient(int width,
                                 int height,
                                 const EXColorConverterSP colorConverter,
                                 const KoColorSpace *generationColorSpace,
                                 const KisDisplayColorConverter *displayColorConverter,
                                 std::function<QVector4D(float, float)> pixelGet,
                                 bool useParallel = true);

void saturateColor(QVector3D &color);
float getRingValue(QPointF widgetCoordCentered, float rotationOffset);
QString colorToString(QVector3D color);
QVector3D stringToColor(const QString &str);
template<typename T>
QString vectorToString(const QVector<T> &vec, std::function<QString(const T &)> toStringFunc)
{
    QStringList parts;
    for (const T &v : vec) {
        parts.append(toStringFunc(v));
    }
    return parts.join(',');
}
template<typename T>
QVector<T> stringToVector(const QString &str, std::function<T(const QString &)> fromStringFunc)
{
    QStringList parts = str.split(',', Qt::SkipEmptyParts);
    QVector<T> vec;
    for (const QString &part : parts) {
        vec.append(fromStringFunc(part.trimmed()));
    }
    return vec;
}
QColor getContrastingColor(const QColor &color);
bool testFlag(int flags, int flag);
} // namespace ExtendedUtils

#endif // EXTENDEDUTILS_H
