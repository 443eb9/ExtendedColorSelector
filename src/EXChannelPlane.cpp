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
#include "EXGamutClipping.h"
#include "EXSettingsState.h"
#include "EXUtils.h"

EXChannelPlane::EXChannelPlane(QWidget *parent)
    : EXEditable(parent)
    , m_shape(nullptr)
    , m_dri(nullptr)
    , m_lastPrimaryChannelValue(-1.0f)
    , m_primaryChannelIndex(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100);
}

void EXChannelPlane::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_dri = canvas->displayColorConverter()->displayRendererInterface();
    }
}

void EXChannelPlane::setColorModel(ColorModelSP colorModel)
{
    m_colorModel = colorModel;
    updateImage();
}

void EXChannelPlane::setPrimaryChannelIndex(int index)
{
    m_primaryChannelIndex = index;
    updateImage();
}

void EXChannelPlane::setColor(QVector3D color)
{
    m_color = color;
    switch (m_primaryChannelIndex) {
    case 0:
        m_secondaryChannelValues = QVector2D(m_color[1], m_color[2]);
        break;
    case 1:
        m_secondaryChannelValues = QVector2D(m_color[0], m_color[2]);
        break;
    case 2:
        m_secondaryChannelValues = QVector2D(m_color[0], m_color[1]);
        break;
    }

    if (m_lastPrimaryChannelValue != color[m_primaryChannelIndex]) {
        m_lastPrimaryChannelValue = color[m_primaryChannelIndex];
        updateImage();
    }
}

void EXChannelPlane::setClipToSrgbGamut(bool clip)
{
    m_clipToSrgbGamut = clip;
    updateImage();
}

void EXChannelPlane::setColorfulRing(bool colorful)
{
    m_colorfulRing = colorful;
    updateImage();
}

void EXChannelPlane::setKoColorConverter(EXColorConverterSP colorConverter)
{
    m_koColorConverter = colorConverter;
    updateImage();
}

void EXChannelPlane::setShape(EXChannelPlaneShapeSP shape)
{
    m_shape = shape;
    m_unnormalizedRing = shape->ring;
    updateNormalizedRing();
    updateImage();
}

void EXChannelPlane::setSanitizeOutOfGamut(bool sanitize, QVector3D outOfGamutColor)
{
    m_sanitizeOutOfGamut = sanitize;
    m_outOfGamutColor = outOfGamutColor;
    updateImage();
}

ColorModelSP EXChannelPlane::colorModel() const
{
    return m_colorModel;
}

void EXChannelPlane::updateNormalizedRing()
{
    if (!m_shape) {
        return;
    }
    float s = size();
    m_shape->ring.margin = m_unnormalizedRing.margin / s * 2;
    m_shape->ring.thickness = m_unnormalizedRing.thickness / s * 2;
    m_shape->ring.rotationOffset = m_unnormalizedRing.rotationOffset;
    m_shape->ring.reversed = m_unnormalizedRing.reversed;
}

void EXChannelPlane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateNormalizedRing();
    updateImage();
}

void EXChannelPlane::paintEvent(QPaintEvent *event)
{
    if (m_image.isNull() || !m_shape) {
        return;
    }

    QWidget::paintEvent(event);
    QPainter painter(this);

    auto offset = (QSize(width(), height()) - m_image.size()) * 0.5;
    painter.drawImage(offset.width(), offset.height(), m_image);

    int size = this->size();
    QVector2D planeValues = m_secondaryChannelValues;

    // auto contrastColor =
    //     ExtendedUtils::getContrastingColor(m_colorModel->transferTo(&SRGBModel(), m_color));
    auto contrastColor = Qt::black; // Placeholder until m_color is properly set TODO
    painter.setPen(QPen(contrastColor, 1));

    if (!m_colorModel->isSrgbBased() && m_clipToSrgbGamut) {
        planeValues = EXGamutClipping::instance()->unmapAxesFromLimited(m_colorModel->id(),
                                                                        m_primaryChannelIndex,
                                                                        m_color[m_primaryChannelIndex],
                                                                        planeValues);
    }

    if (planeValues.x() >= 0.0f && planeValues.x() <= 1.0f && planeValues.y() >= 0.0f && planeValues.y() <= 1.0f) {
        QPointF widgetCoord = m_shape->shapeToWidget01(QPointF(planeValues.x(), planeValues.y()));
        offsetWidgetCoord(widgetCoord);
        painter.drawArc(QRectF(widgetCoord.x() * size - 4, widgetCoord.y() * size - 4, 8, 8), 0, 360 * 16);
    }

    if (m_shape->ring.thickness > 0) {
        QPointF ringWidgetCoord = m_shape->ring.getWidgetCoord(m_color[m_primaryChannelIndex]);
        offsetWidgetCoord(ringWidgetCoord);
        painter.drawArc(QRectF(ringWidgetCoord.x() * size - 4, ringWidgetCoord.y() * size - 4, 8, 8), 0, 360 * 16);
    }
}

void EXChannelPlane::updateImage()
{
    if (!m_dri || !m_shape) {
        m_image = QImage();
        return;
    }

    int alphaPos = m_koColorConverter->colorSpace()->alphaPos();

    auto pixelGet = [this, alphaPos](float x, float y) -> QVector4D {
        QVector3D color;
        QPointF widgetCoord = QPointF(x * 2 - 1, (1 - y) * 2 - 1);
        float dist = qSqrt(widgetCoord.x() * widgetCoord.x() + widgetCoord.y() * widgetCoord.y());

        if (m_shape->ring.thickness > 0 && dist > m_shape->ring.boundaryDiameter() && dist < 1) {
            float ringValue = m_shape->ring.getRingValue(QPointF(x, y));
            color = m_color;
            color[m_primaryChannelIndex] = ringValue;
            if (m_colorfulRing) {
                m_colorModel->makeColorful(color, m_primaryChannelIndex);
            }
        } else {
            QPointF shapeCoord;
            bool isInShape = m_shape->widgetCenteredToShape(widgetCoord, shapeCoord);
            if (!isInShape) {
                return QVector4D();
            }

            QVector2D axes(shapeCoord);
            if (m_clipToSrgbGamut) {
                axes = EXGamutClipping::instance()->mapAxesToLimited(m_colorModel->id(),
                                                                     m_primaryChannelIndex,
                                                                     m_color[m_primaryChannelIndex],
                                                                     axes);
            }

            float channel1 = axes.x();
            float channel2 = axes.y();

            switch (m_primaryChannelIndex) {
            case 0:
                color[0] = m_color[0], color[1] = channel1, color[2] = channel2;
                break;
            case 1:
                color[0] = channel1, color[1] = m_color[1], color[2] = channel2;
                break;
            case 2:
                color[0] = channel1, color[1] = channel2, color[2] = m_color[2];
                break;
            }
        }

        color = m_colorModel->transferTo(m_koColorConverter->colorModel(), color);
        if (!m_colorModel->isSrgbBased() && m_sanitizeOutOfGamut) {
            ExtendedUtils::sanitizeOutOfGamutColor(color, m_outOfGamutColor);
        }
        auto colorWithAlpha = color.toVector4D();
        colorWithAlpha[alphaPos] = 1.0f;
        return colorWithAlpha;
    };
    m_image = ExtendedUtils::generateGradient(size(), size(), true, m_koColorConverter, m_dri, pixelGet);
    update();
}

void EXChannelPlane::startEdit(QMouseEvent *event, bool isShift)
{
    if (!m_shape) {
        return;
    }

    QPointF widgetCoord = QPointF(event->pos()) / size();
    unoffsetWidgetCoord(widgetCoord);
    QPointF centeredCoord = widgetCoord * 2 - QPointF(1, 1);
    float dist = qSqrt(centeredCoord.x() * centeredCoord.x() + centeredCoord.y() * centeredCoord.y());

    if (m_shape->ring.thickness > 0 && dist > m_shape->ring.marginedBoundaryDiameter()) {
        m_editMode = Ring;
        if (isShift) {
            m_editStartWidgetCoordPx = m_shape->ring.getWidgetCoord(m_color[m_primaryChannelIndex]) * size();
        }
    } else {
        m_editMode = Plane;
        if (isShift) {
            QVector2D values = m_secondaryChannelValues;

            if (!m_colorModel->isSrgbBased() && m_clipToSrgbGamut) {
                values = EXGamutClipping::instance()->unmapAxesFromLimited(m_colorModel->id(),
                                                                           m_primaryChannelIndex,
                                                                           m_color[m_primaryChannelIndex],
                                                                           values);
            }

            auto valuesWidgetCoord = m_shape->shapeToWidget01(QPointF(values.x(), values.y()));
            offsetWidgetCoord(valuesWidgetCoord);
            m_editStartWidgetCoordPx = valuesWidgetCoord * size();
        }
    }

    if (!isShift) {
        handleCursorEdit(widgetCoord);
    }
}

void EXChannelPlane::edit(QMouseEvent *event)
{
    if (!m_shape) {
        return;
    }

    QPointF widgetCoord = QPointF(event->pos()) / size();
    unoffsetWidgetCoord(widgetCoord);
    handleCursorEdit(widgetCoord);
}

void EXChannelPlane::shift(QMouseEvent *event, QVector2D delta)
{
    Q_UNUSED(event);

    if (!m_shape) {
        return;
    }

    QPointF widgetCoord = (m_editStartWidgetCoordPx + QPointF(delta.x(), delta.y())) / size();
    unoffsetWidgetCoord(widgetCoord);
    handleCursorEdit(widgetCoord);
}

float EXChannelPlane::size() const
{
    return qMin(width(), height());
}

void EXChannelPlane::sendPlaneColor(const QPointF &widgetCoord)
{
    if (!m_shape) {
        return;
    }

    QPointF shapeCoord;
    m_shape->widget01ToShape(widgetCoord, shapeCoord);
    shapeCoord.setX(qBound(0.0, shapeCoord.x(), 1.0));
    shapeCoord.setY(qBound(0.0, shapeCoord.y(), 1.0));

    if (!m_colorModel->isSrgbBased() && m_clipToSrgbGamut) {
        QVector2D clipped = EXGamutClipping::instance()->mapAxesToLimited(m_colorModel->id(),
                                                                          m_primaryChannelIndex,
                                                                          m_color[m_primaryChannelIndex],
                                                                          QVector2D(shapeCoord));
        shapeCoord = QPointF(clipped.x(), clipped.y());
    }

    m_secondaryChannelValues = QVector2D(shapeCoord);
    Q_EMIT sigSecondaryChannelsValueSelected(m_secondaryChannelValues);
    update();
}

void EXChannelPlane::sendRingColor(const QPointF &widgetCoord)
{
    if (!m_shape) {
        return;
    }

    float ringValue = m_shape->ring.getRingValue(widgetCoord);
    m_color[m_primaryChannelIndex] = ringValue;
    Q_EMIT sigPrimaryChannelValueSelected(m_color[m_primaryChannelIndex]);
    update();
}

void EXChannelPlane::handleCursorEdit(const QPointF &widgetCoord)
{
    if (!m_shape) {
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

void EXChannelPlane::offsetWidgetCoord(QPointF &widgetCoord)
{
    auto offset = (QSize(width(), height()) - m_image.size()) * 0.5;
    widgetCoord += QPointF(offset.width(), offset.height()) / size();
}

void EXChannelPlane::unoffsetWidgetCoord(QPointF &widgetCoord)
{
    auto offset = (QSize(width(), height()) - m_image.size()) * 0.5;
    widgetCoord -= QPointF(offset.width(), offset.height()) / size();
}
