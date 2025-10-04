#include <kis_canvas2.h>
#include <kis_display_color_converter.h>

#include "EXColorState.h"
#include "EXUtils.h"

static EXColorState *s_instance = nullptr;

EXColorState *EXColorState::instance()
{
    if (!s_instance) {
        s_instance = new EXColorState();
    }
    return s_instance;
}

EXColorState::EXColorState()
    : m_color(1, 1, 1)
    , m_primaryChannelIndex(0)
    , m_colorModel(new RGBModel())
    , m_currentColorSpace(nullptr)
    , m_resourceProvider(nullptr)
    , m_koColorConverter(nullptr)
    , m_blockSync(false)
{
    // TODO load from settings
}

void EXColorState::setColorModel(ColorModelId model)
{
    if (m_colorModel->id() == model) {
        return;
    }

    auto newModel = ColorModelFactory::fromId(model);

    m_color = m_colorModel->transferTo(newModel, m_color, nullptr);
    ExtendedUtils::saturateColor(m_color);
    m_colorModel = newModel;
    Q_EMIT sigColorModelChanged(model);
    Q_EMIT sigColorChanged(m_color);
}

const ColorModelSP EXColorState::colorModel() const
{
    return m_colorModel;
}

void EXColorState::sendToKrita()
{
    RGBModel rgbConverter;
    QVector3D currentRgb = m_colorModel->transferTo(&rgbConverter, m_color, nullptr);
    QVector<float> channels(4, 1);
    channels[0] = currentRgb.x(), channels[1] = currentRgb.y(), channels[2] = currentRgb.z();

    m_blockSync = true;
    m_resourceProvider->setFGColor(m_koColorConverter->displayChannelsToKoColor(channels));
    m_blockSync = false;
}

void EXColorState::syncFromKrita()
{
    if (m_blockSync) {
        return;
    }

    KoColor koColor = m_resourceProvider->fgColor();
    auto channels = m_koColorConverter->koColorToDisplayChannels(koColor);
    QVector3D newColor(channels[0], channels[1], channels[2]);
    m_color = m_kritaColorModel->transferTo(m_colorModel, newColor, nullptr);
    Q_EMIT sigColorChanged(m_color);
}

void EXColorState::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        setColorSpace(canvas->image()->colorSpace());
        m_resourceProvider = canvas->imageView()->resourceProvider();
        m_dri = canvas->displayColorConverter()->displayRendererInterface();

        connect(m_resourceProvider, &KisCanvasResourceProvider::sigFGColorChanged, this, &EXColorState::syncFromKrita);
        connect(canvas->image(), &KisImage::sigColorSpaceChanged, this, [this, canvas]() {
            m_currentColorSpace = canvas->image()->colorSpace();
            syncFromKrita();
        });

        syncFromKrita();
        Q_EMIT sigPrimaryChannelIndexChanged(m_primaryChannelIndex);
        Q_EMIT sigColorModelChanged(m_colorModel->id());
    }
}

qreal EXColorState::primaryChannelValue() const
{
    return m_color[m_primaryChannelIndex];
}

void EXColorState::setPrimaryChannelValue(float value)
{
    m_color[m_primaryChannelIndex] = value;
    Q_EMIT sigColorChanged(m_color);
}

quint32 EXColorState::primaryChannelIndex() const
{
    return m_primaryChannelIndex;
}

void EXColorState::setPrimaryChannelIndex(quint32 index)
{
    Q_ASSERT(index < 3);
    m_primaryChannelIndex = index;
    Q_EMIT sigPrimaryChannelIndexChanged(index);
}

QVector2D EXColorState::secondaryChannelValues() const
{
    switch (m_primaryChannelIndex) {
    case 0:
        return QVector2D(m_color[1], m_color[2]);
    case 1:
        return QVector2D(m_color[0], m_color[2]);
    case 2:
        return QVector2D(m_color[0], m_color[1]);
    default:
        return QVector2D();
    }
}

void EXColorState::setSecondaryChannelValues(const QVector2D &values)
{
    switch (m_primaryChannelIndex) {
    case 0:
        m_color[1] = values.x();
        m_color[2] = values.y();
        break;
    case 1:
        m_color[0] = values.x();
        m_color[2] = values.y();
        break;
    case 2:
        m_color[0] = values.x();
        m_color[1] = values.y();
        break;
    }
    Q_EMIT sigColorChanged(m_color);
}

void EXColorState::setChannel(quint32 index, float value)
{
    Q_ASSERT(index < 3);
    m_color[index] = value;
    Q_EMIT sigColorChanged(m_color);
}

QVector3D EXColorState::color() const
{
    return m_color;
}

KoColor EXColorState::koColor() const
{
    auto kritaColor = m_colorModel->transferTo(m_kritaColorModel, m_color, nullptr);
    auto channels = QVector<float>{kritaColor.x(), kritaColor.y(), kritaColor.z(), 1.0f};
    return m_koColorConverter->displayChannelsToKoColor(channels);
}

QColor EXColorState::qColor() const
{
    return m_dri->toQColor(koColor());
}

void EXColorState::setColor(const QVector3D &color)
{
    m_color = color;
    Q_EMIT sigColorChanged(m_color);
}

const KoColorSpace *EXColorState::colorSpace() const
{
    return m_currentColorSpace;
}

const ColorModelSP EXColorState::kritaColorModel() const
{
    return m_kritaColorModel;
}

void EXColorState::setColorSpace(const KoColorSpace *colorSpace)
{
    m_currentColorSpace = colorSpace;
    m_koColorConverter = new EXColorConverter(colorSpace);
    m_kritaColorModel = ColorModelFactory::fromKoColorSpace(colorSpace);
}
