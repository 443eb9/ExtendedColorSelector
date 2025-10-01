#ifndef EXTENDEDUTILS_H
#define EXTENDEDUTILS_H

#include <QImage>
#include <QVector>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>

namespace ExtendedUtils
{
QImage generateGradient(int width,
                        int height,
                        const KoColorSpace *colorSpace,
                        const KoColorDisplayRendererInterface *dri,
                        std::function<void(float, float, QVector<float> &)> pixelGet);

void sanitizeOutOfGamutColor(QVector3D &color, const QVector3D &outOfGamutColor);
} // namespace ExtendedUtils

#endif // EXTENDEDUTILS_H
