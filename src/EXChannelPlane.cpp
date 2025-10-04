#include <QMouseEvent>
#include <QPainter>
#include <QVector4D>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "EXChannelPlane.h"
#include "EXColorState.h"
#include "EXGamutClipping.h"
#include "EXKoColorConverter.h"
#include "EXSettingsState.h"
#include "EXUtils.h"

EXChannelPlane::EXChannelPlane(EXColorPatchPopup *colorPatchPopup, QWidget *parent)
    : EXEditable(parent)
    , m_dri(nullptr)
    , m_shape(nullptr)
    , m_colorPatchPopup(colorPatchPopup)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100);

    connect(EXColorState::instance(), &EXColorState::sigColorChanged, this, [this]() {
        trySyncRingRotation();
        updateImage();
        update();
    });

    connect(EXColorState::instance(), &EXColorState::sigPrimaryChannelIndexChanged, this, [this]() {
        updateImage();
        update();
    });

    connect(EXColorState::instance(), &EXColorState::sigColorModelChanged, this, [this]() {
        settingsChanged();
        updateImage();
        update();
    });

    connect(EXSettingsState::instance(), &EXSettingsState::sigSettingsChanged, this, &EXChannelPlane::settingsChanged);
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

    if (m_shape) {
        delete m_shape;
    }
    m_shape = EXShapeFactory::fromId(settings.shape);
    m_shape->reverseX = settings.reverseX;
    m_shape->reverseY = settings.reverseY;
    m_shape->setRotation(settings.rotation);
    m_shape->swapAxes = settings.swapAxes;

    if (settings.ringEnabled) {
        m_shape->ring = EXPrimaryChannelRing(settings.ringMargin / size() * 2,
                                             settings.ringThickness / size() * 2,
                                             settings.ringRotation,
                                             settings.ringReversed);
    } else {
        m_shape->ring = EXPrimaryChannelRing();
    }

    trySyncRingRotation();
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
    auto colorState = EXColorState::instance();
    auto settings = EXSettingsState::instance()->settings[colorState->colorModel()->id()];

    if (settings.clipToSrgbGamut) {
        planeValues = EXGamutClipping::instance()->unmapAxesFromLimited(colorState->colorModel()->id(),
                                                                        colorState->primaryChannelIndex(),
                                                                        colorState->primaryChannelValue(),
                                                                        planeValues);
    }

    QPointF widgetCoord = m_shape->shapeToWidget01(QPointF(planeValues.x(), planeValues.y()));
    painter.drawArc(QRectF(widgetCoord.x() * size - 4, widgetCoord.y() * size - 4, 8, 8), 0, 360 * 16);

    if (m_shape->ring.thickness > 0) {
        QPointF ringWidgetCoord = m_shape->ring.getWidgetCoord(EXColorState::instance()->primaryChannelValue());
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
    auto &settings = EXSettingsState::instance()->settings[colorState->colorModel()->id()];
    auto makeColorful = settings.colorfulHueRing;
    auto clipToSrgbGamut = settings.clipToSrgbGamut;

    auto pixelGet = [this, colorState, mapper, makeColorful, clipToSrgbGamut](float x, float y) -> QVector4D {
        QVector3D color;
        QPointF widgetCoord = QPointF(x * 2 - 1, (1 - y) * 2 - 1);
        float dist = qSqrt(widgetCoord.x() * widgetCoord.x() + widgetCoord.y() * widgetCoord.y());
        int primaryChannelIndex = colorState->primaryChannelIndex();

        if (m_shape->ring.thickness > 0 && dist > m_shape->ring.boundaryDiameter() && dist < 1) {
            float ringValue = m_shape->ring.getRingValue(QPointF(x, y));
            color = colorState->color();
            color[primaryChannelIndex] = ringValue;
            if (makeColorful) {
                colorState->colorModel()->makeColorful(color, primaryChannelIndex);
            }
        } else {
            float primary = colorState->primaryChannelValue();
            QPointF shapeCoord;
            bool isInShape = m_shape->widgetCenteredToShape(widgetCoord, shapeCoord);
            if (!isInShape) {
                return QVector4D();
            }

            QVector2D axes(shapeCoord);
            if (clipToSrgbGamut) {
                axes = EXGamutClipping::instance()->mapAxesToLimited(colorState->colorModel()->id(),
                                                                     primaryChannelIndex,
                                                                     primary,
                                                                     axes);
            }

            float channel1 = axes.x();
            float channel2 = axes.y();

            switch (primaryChannelIndex) {
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

        color = colorState->colorModel()->transferTo(colorState->kritaColorModel(), color, nullptr);
        auto &settings = EXSettingsState::instance()->globalSettings;
        if (!colorState->colorModel()->isSrgbBased() && settings.outOfGamutColorEnabled) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, settings.outOfGamutColor);
        }

        return QVector4D(color, 1.0f);
    };
    m_image = ExtendedUtils::generateGradient(size(),
                                              size(),
                                              colorState->colorModel()->parallelGradientGen(),
                                              colorState->colorSpace(),
                                              m_dri,
                                              pixelGet);
}

void EXChannelPlane::mousePressEvent(QMouseEvent *event)
{
    if (m_shape == nullptr) {
        return;
    }

    EXEditable::mousePressEvent(event);
    QPointF widgetCoord = QPointF(event->pos()) / qMin(width(), height());
    QPointF centeredCoord = widgetCoord * 2 - QPointF(1, 1);
    float dist = qSqrt(centeredCoord.x() * centeredCoord.x() + centeredCoord.y() * centeredCoord.y());
    float size = this->size();

    if (m_shape->ring.thickness > 0 && dist > m_shape->ring.boundaryDiameter()) {
        m_editMode = Ring;
        m_editStart = m_shape->ring.getWidgetCoord(EXColorState::instance()->primaryChannelValue()) * size;
        sendRingColor(widgetCoord);
    } else {
        m_editMode = Plane;
        QVector2D values = EXColorState::instance()->secondaryChannelValues();
        m_editStart = m_shape->shapeToWidget01(QPointF(values.x(), values.y())) * size;
        sendPlaneColor(widgetCoord);
    }

    if (m_colorPatchPopup) {
        m_colorPatchPopup->popupAt(mapToGlobal(QPoint()) - QPoint(m_colorPatchPopup->width(), 0));
    }
}

void EXChannelPlane::edit(QMouseEvent *event)
{
    if (m_shape == nullptr) {
        return;
    }

    QPointF widgetCoord = QPointF(event->pos()) / size();
    handleCursorEdit(widgetCoord);
}

void EXChannelPlane::shift(QMouseEvent *event, QVector2D delta)
{
    if (m_shape == nullptr) {
        return;
    }

    QPointF widgetCoord = (m_editStart + QPointF(delta.x(), delta.y())) / this->size();
    handleCursorEdit(widgetCoord);
}

void EXChannelPlane::mouseReleaseEvent(QMouseEvent *event)
{
    EXColorState::instance()->sendToKrita();

    if (m_colorPatchPopup && EXSettingsState::instance()->globalSettings.recordLastColorWhenMouseRelease) {
        m_colorPatchPopup->recordColor();
    }
}

float EXChannelPlane::size() const
{
    return qMin(width(), height());
}

void EXChannelPlane::trySyncRingRotation()
{
    if (m_shape == nullptr) {
        return;
    }

    auto colorState = EXColorState::instance();
    auto settings = EXSettingsState::instance()->settings[colorState->colorModel()->id()];
    if (settings.planeRotateWithRing && m_shape != nullptr) {
        float value = colorState->primaryChannelValue();
        m_shape->setRotation(value * 2.0 * M_PI + settings.rotation);
    }
}

void EXChannelPlane::sendPlaneColor(const QPointF &widgetCoord)
{
    if (m_shape == nullptr) {
        return;
    }

    QPointF shapeCoord;
    m_shape->widget01ToShape(widgetCoord, shapeCoord);
    auto colorState = EXColorState::instance();
    if (EXSettingsState::instance()->settings[colorState->colorModel()->id()].clipToSrgbGamut) {
        QVector2D clipped = EXGamutClipping::instance()->mapAxesToLimited(colorState->colorModel()->id(),
                                                                          colorState->primaryChannelIndex(),
                                                                          colorState->primaryChannelValue(),
                                                                          QVector2D(shapeCoord));
        shapeCoord = QPointF(clipped.x(), clipped.y());
    }

    shapeCoord.setX(qBound(0.0, shapeCoord.x(), 1.0));
    shapeCoord.setY(qBound(0.0, shapeCoord.y(), 1.0));

    colorState->setSecondaryChannelValues(QVector2D(shapeCoord));
}

void EXChannelPlane::sendRingColor(const QPointF &widgetCoord)
{
    if (m_shape == nullptr) {
        return;
    }

    float ringValue = m_shape->ring.getRingValue(widgetCoord);
    EXColorState::instance()->setPrimaryChannelValue(ringValue);
}

void EXChannelPlane::handleCursorEdit(const QPointF &widgetCoord)
{
    if (m_shape == nullptr) {
        return;
    }

    switch (m_editMode) {
    case Ring: {
        sendRingColor(widgetCoord);
        break;
    }
    case Plane: {
        sendPlaneColor(widgetCoord);
        break;
    }
    }
}
