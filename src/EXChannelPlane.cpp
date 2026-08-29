#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QVector4D>
#include <qmath.h>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_display_color_converter.h>

#include "EXChannelPlane.h"
#include "EXGamutClipping.h"
#include "EXSettingsState.h"
#include "EXUtils.h"

EXChannelPlane::EXChannelPlane(QWidget *parent)
    : EXEditableImage(parent)
    , m_shape(nullptr)
    , m_lastPrimaryChannelValue(-1.0f)
    , m_primaryChannelIndex(0)
    , m_dynamicRange(1.0f)
    , m_imageDirty(true)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(100, 100);
    setStretch(false);
}

void EXChannelPlane::setCanvas(KisCanvas2 *canvas)
{
    setDisplayColorConverter(canvas ? canvas->displayColorConverter() : nullptr);

    if (displayColorConverter()) {
        connect(displayColorConverter(),
                &KisDisplayColorConverter::displayConfigurationChanged,
                this,
                &EXChannelPlane::updateImage,
                Qt::UniqueConnection);
    }

    updateImage();
}

void EXChannelPlane::setColorModel(ColorModelSP colorModel)
{
    m_colorModel = colorModel;
    auto channelCount = colorModel->channelCount();
    setVisible(channelCount == 2 || channelCount == 3);
}

void EXChannelPlane::setPrimaryChannelIndex(int index)
{
    m_primaryChannelIndex = index;
}

void EXChannelPlane::setColor(QVector3D color, ColorModelSP colorModel)
{
    m_color = colorModel->transferTo(m_colorModel, color, m_color);
    updateSecondaryChannelValues();

    if (m_lastPrimaryChannelValue != color[m_primaryChannelIndex]) {
        m_lastPrimaryChannelValue = color[m_primaryChannelIndex];
        updateImage();
    }
}

void EXChannelPlane::setClipToSrgbGamut(bool clip)
{
    m_clipToSrgbGamut = clip;
}

void EXChannelPlane::setColorfulRing(bool colorful)
{
    m_colorfulRing = colorful;
}

void EXChannelPlane::setColorConverter(EXColorConverterSP colorConverter)
{
    m_converter = colorConverter;
}

void EXChannelPlane::setShape(EXChannelPlaneShapeSP shape)
{
    m_shape = shape;
    m_unnormalizedRing = shape->ring;
    updateNormalizedRing();
}

void EXChannelPlane::setSanitizeOutOfGamut(bool sanitize, QVector3D outOfGamutColor)
{
    m_sanitizeOutOfGamut = sanitize;
    m_outOfGamutColor = outOfGamutColor;
}

void EXChannelPlane::setDynamicRange(float dynamicRange)
{
    m_dynamicRange = dynamicRange;
}

void EXChannelPlane::setUseHdr(bool use)
{
    setUseGLImage(use);
}

ColorModelSP EXChannelPlane::colorModel() const
{
    return m_colorModel;
}

void EXChannelPlane::updateSecondaryChannelValues()
{
    switch (m_colorModel->channelCount()) {
    case 2:
        m_secondaryChannelValues = m_color.toVector2D();
        break;
    case 3:
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
        break;
    }
}

void EXChannelPlane::setSecondaryChannelValues(QVector2D values)
{
    m_secondaryChannelValues = values;
    switch (m_colorModel->channelCount()) {
    case 2:
        m_color = QVector3D(values.x(), values.y(), 0.0f);
        break;
    case 3:
        switch (m_primaryChannelIndex) {
        case 0:
            m_color = QVector3D(m_color[0], values.x(), values.y());
            break;
        case 1:
            m_color = QVector3D(values.x(), m_color[1], values.y());
            break;
        case 2:
            m_color = QVector3D(values.x(), values.y(), m_color[2]);
            break;
        }
        break;
    }
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
    KisGLImageWidget::resizeEvent(event);
    updateNormalizedRing();
    updateImage();
}

void EXChannelPlane::paintEvent(QPaintEvent *event)
{
    EXEditableImage::paintEvent(event);

    if (!m_shape || !m_converter || !displayRenderer()) {
        return;
    }

    QPainter painter(this);

    QVector2D planeValues = m_secondaryChannelValues;

    QVector3D displayColor = m_colorModel->transferTo(m_converter->colorModel(), m_color);
    displayColor *= m_dynamicRange;
    auto contrastColor = ExtendedUtils::getContrastingColor(
        displayRenderer()->toQColor(m_converter->displayChannelsToKoColor(QVector4D(displayColor, 1.0f))));
    painter.setPen(QPen(contrastColor, 1));

    if (m_clipToSrgbGamut) {
        planeValues = EXGamutClipping::instance()->unmapAxesFromLimited(m_colorModel->id(),
                                                                        m_primaryChannelIndex,
                                                                        m_color[m_primaryChannelIndex],
                                                                        planeValues);
    }

    int size = this->size();
    const float epsilon = 1e-5f;
    if (planeValues.x() > -epsilon && planeValues.x() < 1.0f + epsilon && planeValues.y() > -epsilon
        && planeValues.y() < 1.0f + epsilon) {
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
    if (!isVisible() || !displayRenderer() || !m_shape || !m_converter || !displayColorConverter() || !m_colorModel) {
        m_imageDirty = true;
        return;
    }

    int alphaPos = m_converter->colorSpace()->alphaPos();

    auto pixelGet3 = [this, alphaPos](float x, float y) -> QVector4D {
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

        if (m_sanitizeOutOfGamut) {
            color = m_colorModel->transferToWithGamutWarning(m_converter->colorModel(), color, m_outOfGamutColor);
        } else {
            color = m_colorModel->transferTo(m_converter->colorModel(), color);
        }
        color *= m_dynamicRange;
        auto colorWithAlpha = color.toVector4D();
        colorWithAlpha[alphaPos] = 1.0f;
        return colorWithAlpha;
    };

    auto pixelGet2 = [this, alphaPos](float x, float y) -> QVector4D {
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

            color[0] = axes.x();
            color[1] = axes.y();
        }

        if (m_sanitizeOutOfGamut) {
            color = m_colorModel->transferToWithGamutWarning(m_converter->colorModel(), color, m_outOfGamutColor);
        } else {
            color = m_colorModel->transferTo(m_converter->colorModel(), color);
        }
        color *= m_dynamicRange;
        auto colorWithAlpha = color.toVector4D();
        colorWithAlpha[alphaPos] = 1.0f;
        return colorWithAlpha;
    };

    const KoColorSpace *generationCS = generationColorSpace(m_converter ? m_converter->colorSpace() : nullptr);

    switch (m_colorModel->channelCount()) {
    case 2:
        ExtendedUtils::loadImageIntoEditableWidget(this,
                                                   size(),
                                                   size(),
                                                   true,
                                                   generationCS,
                                                   m_converter,
                                                   displayColorConverter(),
                                                   pixelGet2);
        break;
    case 3:
        ExtendedUtils::loadImageIntoEditableWidget(this,
                                                   size(),
                                                   size(),
                                                   true,
                                                   generationCS,
                                                   m_converter,
                                                   displayColorConverter(),
                                                   pixelGet3);
        break;
    }

    m_imageDirty = false;
    update();
}

void EXChannelPlane::showEvent(QShowEvent *event)
{
    EXEditableImage::showEvent(event);
    if (m_imageDirty) {
        updateImage();
    }
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

            if (m_clipToSrgbGamut) {
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

    if (m_clipToSrgbGamut) {
        QVector2D clipped = EXGamutClipping::instance()->mapAxesToLimited(m_colorModel->id(),
                                                                          m_primaryChannelIndex,
                                                                          m_color[m_primaryChannelIndex],
                                                                          QVector2D(shapeCoord));
        shapeCoord = QPointF(clipped.x(), clipped.y());
    }

    setSecondaryChannelValues(QVector2D(shapeCoord));
    Q_EMIT sigSecondaryChannelsValueSelected(m_secondaryChannelValues, m_color);
    update();
}

void EXChannelPlane::sendRingColor(const QPointF &widgetCoord)
{
    if (!m_shape) {
        return;
    }

    float ringValue = m_shape->ring.getRingValue(widgetCoord);
    m_color[m_primaryChannelIndex] = ringValue;
    Q_EMIT sigPrimaryChannelValueSelected(m_color[m_primaryChannelIndex], m_color);
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
    auto offset = (QSize(width() - size(), height() - size())) * 0.5;
    widgetCoord += QPointF(offset.width(), offset.height()) / size();
}

void EXChannelPlane::unoffsetWidgetCoord(QPointF &widgetCoord)
{
    auto offset = (QSize(width() - size(), height() - size())) * 0.5;
    widgetCoord -= QPointF(offset.width(), offset.height()) / size();
}
