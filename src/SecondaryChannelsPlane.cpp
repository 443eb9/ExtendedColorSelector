#include <QPainter>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "SecondaryChannelsPlane.h"

SecondaryChannelsPlane::SecondaryChannelsPlane(QWidget *parent)
    : QWidget(parent)
    , m_dri(nullptr)
    , m_shape(new SquareShape())
    , m_colorState(ColorState::instance())
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SecondaryChannelsPlane::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
        updateImage();
        update();
    }
}

void SecondaryChannelsPlane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void SecondaryChannelsPlane::updateImage()
{
    quint32 size = qMin(width(), height());

    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceSize = qCeil(size * deviceDivider);
    const qsizetype pixelSize = m_colorState->colorSpace()->pixelSize();
    quint32 imageSize = deviceSize * deviceSize * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();
    RgbConverter rgbConverter;

    KoColor filler(Qt::white, m_colorState->colorSpace());
    for (int y = 0; y < deviceSize; y++) {
        for (int x = 0; x < deviceSize; x++) {
            // Inverted y.
            QPointF widgetCoord = QPointF((qreal)x / (size - 1), 1 - (qreal)y / (size - 1));
            QPointF shapeCoord = m_shape->widgetToShapeCoordinate(widgetCoord);
            QVector3D color;
            qreal primary = m_colorState->primaryChannelValue();

            switch (m_colorState->primaryChannelIndex()) {
            case 0:
                color = QVector3D(primary, shapeCoord.x(), shapeCoord.y());
                break;
            case 1:
                color = QVector3D(shapeCoord.x(), primary, shapeCoord.y());
                break;
            case 2:
                color = QVector3D(shapeCoord.x(), shapeCoord.y(), primary);
                break;
            }

            KoColor koColor(QColor::fromRgbF(color.x(), color.y(), color.z()), m_colorState->colorSpace());
            memcpy(dataPtr, koColor.data(), pixelSize);
            dataPtr += pixelSize;
        }
    }

    QImage image = m_dri->toQImage(m_colorState->colorSpace(), raw.data(), QSize(deviceSize, deviceSize), false);
    image.setDevicePixelRatio(devicePixelRatioF());

    m_image = image;
}

void SecondaryChannelsPlane::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    if (m_image.isNull() || m_image.size() != size()) {
        updateImage();
    }

    painter.drawImage(0, 0, m_image);
}
