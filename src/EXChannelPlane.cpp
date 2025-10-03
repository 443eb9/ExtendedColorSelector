#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "EXChannelPlane.h"
#include "EXColorState.h"
#include "EXKoColorConverter.h"
#include "EXSettingsState.h"
#include "EXUtils.h"

EXChannelPlane::EXChannelPlane(QWidget *parent)
    : EXEditable(parent)
    , m_dri(nullptr)
    , m_shape(nullptr)
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

    connect(EXColorState::instance(), &EXColorState::sigColorModelChanged, [this]() {
        updateImage();
        update();
    });

    connect(EXSettingsState::instance(), &EXSettingsState::settingsChanged, this, &EXChannelPlane::settingsChanged);
}

void EXChannelPlane::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
    }
}

void EXChannelPlane::settingsChanged()
{
    auto &settings = EXSettingsState::instance()->settings[EXColorState::instance()->colorModel()->id()];
    if (settings.ringEnabled) {
        m_ring.margin = settings.ringMargin / size() * 2;
        m_ring.thickness = settings.ringThickness / size() * 2;
        m_ring.rotationOffset = settings.ringRotation;
    } else {
        m_ring.thickness = 0;
    }

    if (m_shape) {
        delete m_shape;
    }
    m_shape = EXShapeFactory::fromId(settings.shape);

    updateImage();
    update();
}

void EXChannelPlane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateImage();
}

void EXChannelPlane::paintEvent(QPaintEvent *event)
{
    if (m_image.isNull() || m_shape == nullptr) {
        return;
    }

    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.drawImage(0, 0, m_image);

    QVector2D planeValues = EXColorState::instance()->secondaryChannelValues();
    int size = this->size();
    QPointF widgetCoord = m_shape->shapeToWidgetCoord(QPointF(planeValues.x(), planeValues.y()), m_ring);
    painter.drawArc(QRectF(widgetCoord.x() * size - 4, widgetCoord.y() * size - 4, 8, 8), 0, 360 * 16);

    if (m_ring.thickness > 0) {
        QPointF ringWidgetCoord = currentRingWidgetCoord();
        painter.drawArc(QRectF(ringWidgetCoord.x() * size - 4, ringWidgetCoord.y() * size - 4, 8, 8), 0, 360 * 16);
    }
}

void EXChannelPlane::updateImage()
{
    if (m_dri == nullptr || m_shape == nullptr) {
        m_image = QImage();
        return;
    }

    auto colorState = EXColorState::instance();
    auto converter = EXColorConverter(colorState->colorSpace());
    auto mapper = converter.displayToMemoryPositionMapper();

    auto pixelGet = [this, colorState, mapper](float x, float y, QVector<float> &channels) {
        QVector3D color;
        QPointF widgetCoord = QPointF(x * 2 - 1, (1 - y) * 2 - 1);
        float dist = qSqrt(widgetCoord.x() * widgetCoord.x() + widgetCoord.y() * widgetCoord.y());
        if (m_ring.thickness > 0 && dist > m_ring.boundaryDiameter() && dist < 1) {
            float ringValue = m_ring.getRingValue(QPointF(x, y));
            color = colorState->color();
            color[colorState->primaryChannelIndex()] = ringValue;
        } else {
            float boundaryDiameter = m_ring.marginedBoundaryDiameter();
            QPointF shapeCoord;
            bool isInShape = m_shape->widgetCenteredToShapeCoord(widgetCoord, shapeCoord, m_ring);
            float channel1 = shapeCoord.x();
            float channel2 = shapeCoord.y();
            if (!isInShape) {
                channels[mapper[3]] = 0;
                return;
            }

            float primary = colorState->primaryChannelValue();

            switch (colorState->primaryChannelIndex()) {
            case 0:
                color[0] = primary, color[1] = channel1, color[2] = channel2;
                break;
            case 1:
                color[0] = channel1, color[1] = primary, color[2] = channel2;
                break;
            case 2:
                color[0] = channel1, color[1] = channel2, color[2] = primary;
                break;
            }
        }

        color = colorState->kritaColorModel()->fromXyz(colorState->colorModel()->toXyz(color));
        if (!colorState->colorModel()->isSrgbBased()) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, QVector3D(0.5, 0.5, 0.5));
        }
        channels[mapper[0]] = color[0], channels[mapper[1]] = color[1], channels[mapper[2]] = color[2];
        channels[mapper[3]] = 1;
    };
    m_image = ExtendedUtils::generateGradient(size(), size(), colorState->colorSpace(), m_dri, pixelGet);
}

void EXChannelPlane::mousePressEvent(QMouseEvent *event)
{
    if (m_shape == nullptr) {
        return;
    }

    EXEditable::mousePressEvent(event);
    QPointF widgetCoord = QPointF(event->pos()) / qMin(width(), height()) * 2 - QPointF(1, 1);
    float dist = qSqrt(widgetCoord.x() * widgetCoord.x() + widgetCoord.y() * widgetCoord.y());
    float size = this->size();

    if (dist > m_ring.boundaryDiameter()) {
        m_editMode = Ring;
        m_editStart = currentRingWidgetCoord() * size;
    } else {
        m_editMode = Plane;
        QVector2D values = EXColorState::instance()->secondaryChannelValues();
        m_editStart = m_shape->shapeToWidgetCoord(QPointF(values.x(), values.y()), m_ring) * size;
    }
}

void EXChannelPlane::edit(QMouseEvent *event)
{
    if (m_shape == nullptr) {
        return;
    }

    QPointF widgetCoord = QPointF(event->pos()) / size();

    switch (m_editMode) {
    case Ring: {
        float ringValue = m_ring.getRingValue(widgetCoord);
        EXColorState::instance()->setPrimaryChannelValue(ringValue);
        break;
    }
    case Plane: {
        QPointF shapeCoord;
        m_shape->widgetToShapeCoord(widgetCoord, shapeCoord, m_ring);

        shapeCoord.setX(qBound(0.0, shapeCoord.x(), 1.0));
        shapeCoord.setY(qBound(0.0, shapeCoord.y(), 1.0));

        EXColorState::instance()->setSecondaryChannelValues(QVector2D(shapeCoord));
        break;
    }
    }
}

void EXChannelPlane::shift(QMouseEvent *event, QVector2D delta)
{
    if (m_shape == nullptr) {
        return;
    }

    QPointF widgetCoord = (m_editStart + QPointF(delta.x(), delta.y())) / this->size();

    switch (m_editMode) {
    case Ring: {
        float ringValue = m_ring.getRingValue(widgetCoord);
        EXColorState::instance()->setPrimaryChannelValue(ringValue);
        break;
    }
    case Plane: {
        QPointF shapeCoord;
        m_shape->widgetToShapeCoord(widgetCoord, shapeCoord, m_ring);

        shapeCoord.setX(qBound(0.0, shapeCoord.x(), 1.0));
        shapeCoord.setY(qBound(0.0, shapeCoord.y(), 1.0));

        EXColorState::instance()->setSecondaryChannelValues(QVector2D(shapeCoord));
    }
    }
}

void EXChannelPlane::mouseReleaseEvent(QMouseEvent *event)
{
    EXColorState::instance()->sendToKrita();
}

float EXChannelPlane::size() const
{
    return qMin(width(), height());
}

QPointF EXChannelPlane::currentRingWidgetCoord()
{
    return m_ring.getWidgetCoord(EXColorState::instance()->primaryChannelValue());
}
