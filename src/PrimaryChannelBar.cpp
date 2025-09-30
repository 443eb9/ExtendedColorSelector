#include <QMouseEvent>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <QPainter>
#include <kis_display_color_converter.h>

#include "ColorState.h"
#include "PrimaryChannelBar.h"

PrimaryChannelBar::PrimaryChannelBar(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(20);

    connect(ColorState::instance().data(), &ColorState::sigColorChanged, this, [this]() {
        updateImage();
        update();
    });
}

void PrimaryChannelBar::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
        updateImage();
        update();
    }
}

void PrimaryChannelBar::updateImage()
{
    ColorStateSP colorState = ColorState::instance();

    const qreal deviceDivider = 1.0 / devicePixelRatioF();
    const int deviceSize = qCeil(width() * deviceDivider);
    const qsizetype pixelSize = colorState->colorSpace()->pixelSize();
    quint32 imageSize = deviceSize * deviceSize * pixelSize;
    QScopedArrayPointer<quint8> raw(new quint8[imageSize]{});
    quint8 *dataPtr = raw.data();
    RgbConverter rgbConverter;

    for (int y = 0; y < deviceSize; y++) {
        for (int x = 0; x < deviceSize; x++) {
            qreal value = (qreal)x / (width() - 1);
            QVector3D color;
            QVector2D secondaryValues = colorState->secondaryChannelValues();

            switch (colorState->primaryChannelIndex()) {
            case 0:
                color = QVector3D(value, secondaryValues.x(), secondaryValues.y());
                break;
            case 1:
                color = QVector3D(secondaryValues.x(), value, secondaryValues.y());
                break;
            case 2:
                color = QVector3D(secondaryValues.x(), secondaryValues.y(), value);
                break;
            }

            KoColor koColor(QColor::fromRgbF(color.x(), color.y(), color.z()), colorState->colorSpace());
            memcpy(dataPtr, koColor.data(), pixelSize);
            dataPtr += pixelSize;
        }
    }

    QImage image = m_dri->toQImage(colorState->colorSpace(), raw.data(), QSize(deviceSize, deviceSize), false);
    image.setDevicePixelRatio(devicePixelRatioF());

    m_image = image;
}

void PrimaryChannelBar::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.drawImage(0, 0, m_image, 0, 0, -1, height());
}

void PrimaryChannelBar::mousePressEvent(QMouseEvent *event)
{
}

void PrimaryChannelBar::mouseMoveEvent(QMouseEvent *event)
{
    qreal value = qBound(0, event->pos().x() / width(), 1);
    ColorState::instance()->setPrimaryChannelValue(value);
}

void PrimaryChannelBar::mouseReleaseEvent(QMouseEvent *event)
{
    ColorState::instance()->sendToKrita();
}
