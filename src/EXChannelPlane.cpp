#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "EXColorState.h"
#include "EXChannelPlane.h"
#include "EXKoColorConverter.h"
#include "EXUtils.h"

EXChannelPlane::EXChannelPlane(QWidget *parent)
    : QWidget(parent)
    , m_dri(nullptr)
    , m_shape(new EXSquareChannelPlaneShape())
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100);

    connect(EXColorState::instance(), &EXColorState::sigColorChanged, [this]() {
        updateImage();
        update();
    });

    connect(EXColorState::instance(), &EXColorState::sigPrimaryChannelIndexChanged, [this]() {
        updateImage();
        update();
    });
}

void EXChannelPlane::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
    }
}

void EXChannelPlane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateImage();
}

void EXChannelPlane::updateImage()
{
    if (m_dri == nullptr) {
        m_image = QImage();
        return;
    }

    int size = qMin(width(), height());
    auto colorState = EXColorState::instance();
    auto converter = EXColorConverter(colorState->colorSpace());
    auto mapper = converter.displayToMemoryPositionMapper();

    auto pixelGet = [this, colorState, mapper](float x, float y, QVector<float> &channels) {
        QPointF widgetCoord = QPointF(x, 1 - y);
        QPointF shapeCoord = m_shape->widgetToShapeCoord(widgetCoord);
        float primary = colorState->primaryChannelValue();
        float channel1 = shapeCoord.x();
        float channel2 = shapeCoord.y();

        switch (colorState->primaryChannelIndex()) {
        case 0:
            channels[mapper[0]] = primary, channels[mapper[1]] = channel1, channels[mapper[2]] = channel2;
            break;
        case 1:
            channels[mapper[0]] = channel1, channels[mapper[1]] = primary, channels[mapper[2]] = channel2;
            break;
        case 2:
            channels[mapper[0]] = channel1, channels[mapper[1]] = channel2, channels[mapper[2]] = primary;
            break;
        }
        channels[mapper[3]] = 1;
    };
    m_image = ExtendedUtils::generateGradient(size, size, colorState->colorSpace(), m_dri, pixelGet);
}

void EXChannelPlane::mousePressEvent(QMouseEvent *event)
{
    m_editMode = Plane;
}

void EXChannelPlane::mouseMoveEvent(QMouseEvent *event)
{
    int size = qMin(width(), height());
    QPointF widgetCoord = QPointF(event->pos()) / size;
    widgetCoord.setX(qBound(0.0, widgetCoord.x(), 1.0));
    widgetCoord.setY(qBound(0.0, widgetCoord.y(), 1.0));
    QPointF shapePos = m_shape->widgetToShapeCoord(widgetCoord);
    EXColorState::instance()->setSecondaryChannelValues(QVector2D(shapePos.x(), 1 - shapePos.y()));
}

void EXChannelPlane::mouseReleaseEvent(QMouseEvent *event)
{
    EXColorState::instance()->sendToKrita();
}

void EXChannelPlane::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.drawImage(0, 0, m_image);

    QVector2D planeValues = EXColorState::instance()->secondaryChannelValues();
    int size = qMin(width(), height());
    QPointF cursorPos = m_shape->shapeToWidgetCoord(QPointF(planeValues.x(), 1 - planeValues.y()));
    painter.drawArc(QRectF(cursorPos.x() * size - 4, cursorPos.y() * size - 4, 8, 8), 0, 360 * 16);
}
