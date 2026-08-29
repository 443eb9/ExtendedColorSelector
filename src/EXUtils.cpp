#include <QPoint>
#include <QRect>
#include <QSize>
#include <QVector>
#include <QtConcurrent>
#include <cstring>
#include <numeric>
#include <qmath.h>

#include "EXColorModel.h"
#include "EXKoColorConverter.h"
#include "EXUtils.h"

#include <KisDisplayConfig.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include "kis_fixed_paint_device.h"
#include <kis_display_color_converter.h>

namespace ExtendedUtils
{
void loadImageIntoEditableWidget(EXEditableImage *editable,
                                 int width,
                                 int height,
                                 bool useParallel,
                                 const KoColorSpace *generationColorSpace,
                                 const EXColorConverterSP colorConverter,
                                 const KisDisplayColorConverter *displayConverter,
                                 std::function<QVector4D(float, float)> pixelGet)
{
    QImage image = generateGradient(width,
                                    height,
                                    useParallel,
                                    colorConverter,
                                    displayConverter->displayRendererInterface(),
                                    pixelGet);
    editable->loadQImage(image);
    // if (editable->useGLImage()) {
    //     KisGLImageF16 image = generateGLGradient(width,
    //                                              height,
    //                                              colorConverter,
    //                                              generationColorSpace,
    //                                              displayConverter,
    //                                              pixelGet,
    //                                              useParallel);
    //     editable->loadGLImage(image);
    // } else {
    //     QImage image = generateGradient(width,
    //                                     height,
    //                                     useParallel,
    //                                     colorConverter,
    //                                     displayConverter->displayRendererInterface(),
    //                                     pixelGet);
    //     editable->loadQImage(image);
    // }
}

QImage generateGradient(int width,
                        int height,
                        bool useParallel,
                        const EXColorConverterSP colorConverter,
                        const KoColorDisplayRendererInterface *dri,
                        std::function<QVector4D(float, float)> pixelGet)
{
    const int deviceWidth = qCeil(width);
    const int deviceHeight = qCeil(height);
    const KoColorSpace *colorSpace = colorConverter->colorSpace();
    const qsizetype pixelSize = colorSpace->pixelSize();
    quint32 imageSize = deviceWidth * deviceHeight * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();

    auto processRow = [&](int y) {
        QVector<float> tempChannelBuffer(colorConverter->colorSpace()->channelCount());
        quint8 *rowPtr = dataPtr + y * deviceWidth * pixelSize;

        for (int x = 0; x < deviceWidth; x++) {
            auto channels = pixelGet((float)x / (width - 1), (float)y / (height - 1));
            colorConverter->displayChannelsToKoColor(rowPtr, channels, tempChannelBuffer);
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

KisGLImageF16 generateGLGradient(int width,
                                 int height,
                                 const EXColorConverterSP colorConverter,
                                 const KoColorSpace *generationColorSpace,
                                 const KisDisplayColorConverter *displayColorConverter,
                                 std::function<QVector4D(float, float)> pixelGet,
                                 bool useParallel)
{
    if (width <= 0 || height <= 0 || !colorConverter || !generationColorSpace || !displayColorConverter) {
        return KisGLImageF16();
    }

    KisFixedPaintDeviceSP device = new KisFixedPaintDevice(generationColorSpace);
    device->setRect(QRect(QPoint(), QSize(width, height)));
    device->reallocateBufferWithoutInitialization();

    quint8 *deviceBytePtr = device->data();
    const qsizetype pixelSize = generationColorSpace->pixelSize();
    const qsizetype rowStrideBytes = pixelSize * width;

    const KoColorSpace *converterSpace = colorConverter->colorSpace();
    const int converterChannelCount = converterSpace->channelCount();

    const float invWidthMinusOne = width > 1 ? 1.0f / (width - 1) : 0.0f;
    const float invHeightMinusOne = height > 1 ? 1.0f / (height - 1) : 0.0f;
    const bool parallelRows = useParallel && height > 1;

    QVector<int> rowIndices;
    if (parallelRows) {
        rowIndices.resize(height);
        std::iota(rowIndices.begin(), rowIndices.end(), 0);
    }

    auto processRow = [&](int y) {
        quint8 *rowBytePtr = deviceBytePtr + rowStrideBytes * y;

        KoColor localSrc(converterSpace);
        KoColor localDst(generationColorSpace);
        static thread_local QVector<float> tempChannelsStorage;
        tempChannelsStorage.resize(converterChannelCount);
        QVector<float> &tempChannels = tempChannelsStorage;
        const float v = y * invHeightMinusOne;

        for (int x = 0; x < width; ++x) {
            const float u = x * invWidthMinusOne;
            const QVector4D displayChannels = pixelGet(u, v);

            colorConverter->displayChannelsToKoColor(localSrc.data(), displayChannels, tempChannels);
            localDst.fromKoColor(localSrc);

            std::memcpy(rowBytePtr, localDst.data(), static_cast<size_t>(pixelSize));
            rowBytePtr += pixelSize;
        }
    };

    if (parallelRows) {
        QtConcurrent::blockingMap(rowIndices, processRow);
    } else {
        for (int y = 0; y < height; ++y) {
            processRow(y);
        }
    }

    const KoColorProfile *outputProfile = displayColorConverter->displayConfig().profile;
    if (!outputProfile) {
        outputProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    }
    const KoColorSpace *outputColorSpace = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                        Float16BitsColorDepthID.id(),
                                                                                        outputProfile);
    displayColorConverter->applyDisplayFilteringF32(device, outputColorSpace);

    KisGLImageF16 image(QSize(width, height));
    const KoColorSpace *outputSpace = device->colorSpace();
    const int outputChannels = outputSpace ? outputSpace->channelCount() : 0;
    Q_ASSERT(outputChannels == 4);

    const size_t bytesToCopy = static_cast<size_t>(width) * height * outputChannels * sizeof(half);
    std::memcpy(image.data(), device->constData(), bytesToCopy);

    return image;
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

QColor getContrastingColor(const QColor &color)
{
    double luminance = 0.2126 * color.redF() + 0.7152 * color.greenF() + 0.0722 * color.blueF();
    return luminance > 0.5 ? QColor(Qt::black) : QColor(Qt::white);
}

bool testFlag(int flags, int flag)
{
    return ((flags >> flag) & 1) == 1;
}
} // namespace ExtendedUtils
