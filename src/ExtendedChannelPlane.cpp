#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "ColorState.h"
#include "ExtendedChannelPlane.h"

ExtendedChannelPlane::ExtendedChannelPlane(QWidget *parent)
    : QWidget(parent)
    , m_dri(nullptr)
    , m_shape(new SquareShape())
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100);

    connect(ColorState::instance(), &ColorState::sigColorChanged, [this]() {
        updateImage();
        update();
    });

    connect(ColorState::instance(), &ColorState::sigPrimaryChannelIndexChanged, [this]() {
        updateImage();
        update();
    });
}

void ExtendedChannelPlane::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
    }
}

void ExtendedChannelPlane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateImage();
}

void ExtendedChannelPlane::updateImage()
{
    if (m_dri == nullptr) {
        m_image = QImage();
        return;
    }

    int size = qMin(width(), height());
    auto colorState = ColorState::instance();

    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceSize = qCeil(size * deviceDivider);
    const qsizetype pixelSize = colorState->colorSpace()->pixelSize();
    quint32 imageSize = deviceSize * deviceSize * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();
    RGBModel rgbConverter;

    QVector<float> channels(4, 1);
    KoColor color(colorState->colorSpace());
    for (int y = 0; y < deviceSize; y++) {
        for (int x = 0; x < deviceSize; x++) {
            // Inverted y.
            QPointF widgetCoord = QPointF((qreal)x / (size - 1), 1 - (qreal)y / (size - 1));
            QPointF shapeCoord = m_shape->widgetToShapeCoord(widgetCoord);
            float primary = colorState->primaryChannelValue();
            float channel1 = shapeCoord.x();
            float channel2 = shapeCoord.y();

            switch (colorState->primaryChannelIndex()) {
            case 0:
                channels[0] = primary, channels[1] = channel1, channels[2] = channel2;
                break;
            case 1:
                channels[0] = channel1, channels[1] = primary, channels[2] = channel2;
                break;
            case 2:
                channels[0] = channel1, channels[1] = channel2, channels[2] = primary;
                break;
            }

            color.colorSpace()->fromNormalisedChannelsValue(color.data(), channels);
            memcpy(dataPtr, color.data(), pixelSize);
            dataPtr += pixelSize;
        }
    }

    QImage image = m_dri->toQImage(colorState->colorSpace(), raw.data(), QSize(deviceSize, deviceSize), false);
    image.setDevicePixelRatio(devicePixelRatioF());

    m_image = image;
}

void ExtendedChannelPlane::mousePressEvent(QMouseEvent *event)
{
    m_editMode = Plane;
}

void ExtendedChannelPlane::mouseMoveEvent(QMouseEvent *event)
{
    int size = qMin(width(), height());
    QPointF widgetCoord = QPointF(event->pos()) / size;
    widgetCoord.setX(qBound(0.0, widgetCoord.x(), 1.0));
    widgetCoord.setY(qBound(0.0, widgetCoord.y(), 1.0));
    QPointF shapePos = m_shape->widgetToShapeCoord(widgetCoord);
    ColorState::instance()->setSecondaryChannelValues(QVector2D(shapePos.x(), 1 - shapePos.y()));
}

void ExtendedChannelPlane::mouseReleaseEvent(QMouseEvent *event)
{
    ColorState::instance()->sendToKrita();
}

void ExtendedChannelPlane::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.drawImage(0, 0, m_image);

    QVector2D planeValues = ColorState::instance()->secondaryChannelValues();
    int size = qMin(width(), height());
    QPointF cursorPos = m_shape->shapeToWidgetCoord(QPointF(planeValues.x(), 1 - planeValues.y()));
    painter.drawArc(QRectF(cursorPos.x() * size - 4, cursorPos.y() * size - 4, 8, 8), 0, 360 * 16);
}
