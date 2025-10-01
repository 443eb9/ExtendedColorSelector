#include "ColorState.h"

static ColorState *s_instance = nullptr;

ColorState *ColorState::instance()
{
    if (!s_instance) {
        s_instance = new ColorState();
    }
    return s_instance;
}

ColorState::ColorState()
    : m_color(1, 1, 1)
    , m_primaryChannelIndex(0)
    , m_colorModel(new RGBModel())
    , m_currentColorSpace(nullptr)
    , m_resourceProvider(nullptr)
    , m_koColorConverter(nullptr)
{
    // TODO load from settings
}

void ColorState::setColorModel(ColorModelId model)
{
    switch (model) {
    case ColorModelId::Rgb:
        m_colorModel = ColorModelSP(new RGBModel());
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

ColorModelSP ColorState::colorModel() const
{
    return m_colorModel;
}

void ColorState::sendToKrita()
{
    RGBModel rgbConverter;
    QVector3D currentRgb = rgbConverter.fromXyz(m_colorModel->toXyz(m_color));
    QVector<float> channels(4, 1);
    channels[0] = currentRgb.x(), channels[1] = currentRgb.y(), channels[2] = currentRgb.z();

    m_resourceProvider->setFGColor(m_koColorConverter->displayChannelsToKoColor(channels));
}

void ColorState::syncFromKrita()
{
    KoColor color = m_resourceProvider->fgColor();
    auto channels = m_koColorConverter->koColorToDisplayChannels(color);
    for (int i = 0; i < 3; i++) {
        m_color[i] = channels[i];
    }
}

void ColorState::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_currentColorSpace = canvas->image()->colorSpace();
        m_koColorConverter = new ExtendedColorConverter(m_currentColorSpace);
        m_resourceProvider = canvas->imageView()->resourceProvider();

        connect(m_resourceProvider, &KisCanvasResourceProvider::sigFGColorChanged, this, &ColorState::syncFromKrita);
        connect(canvas->image(), &KisImage::sigColorSpaceChanged, this, [this, canvas]() {
            m_currentColorSpace = canvas->image()->colorSpace();
            syncFromKrita();
        });

        syncFromKrita();
        Q_EMIT sigPrimaryChannelIndexChanged(m_primaryChannelIndex);
    }
}

qreal ColorState::primaryChannelValue() const
{
    return m_color[m_primaryChannelIndex];
}

void ColorState::setPrimaryChannelValue(qreal value)
{
    m_color[m_primaryChannelIndex] = value;
    setColor(m_color);
}

quint32 ColorState::primaryChannelIndex() const
{
    return m_primaryChannelIndex;
}

void ColorState::setPrimaryChannelIndex(quint32 index)
{
    Q_ASSERT(index < 3);
    m_primaryChannelIndex = index;
    Q_EMIT sigPrimaryChannelIndexChanged(index);
}

QVector2D ColorState::secondaryChannelValues() const
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

void ColorState::setSecondaryChannelValues(const QVector2D &values)
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
    setColor(m_color);
}

void ColorState::setChannel(quint32 index, qreal value)
{
    Q_ASSERT(index < 3);
    m_color[index] = value;
    setColor(m_color);
}

QVector3D ColorState::color() const
{
    return m_color;
}

void ColorState::setColor(const QVector3D &color)
{
    m_color = color;
    Q_EMIT sigColorChanged(m_color);
}

const KoColorSpace *ColorState::colorSpace() const
{
    return m_currentColorSpace;
}

void ColorState::setColorSpace(const KoColorSpace *colorSpace)
{
    m_currentColorSpace = colorSpace;
}
