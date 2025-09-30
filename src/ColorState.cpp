#include "ColorState.h"

static ColorStateSP s_instance = nullptr;

ColorStateSP ColorState::instance()
{
    if (!s_instance) {
        s_instance = ColorStateSP(new ColorState());
    }
    return s_instance;
}

ColorState::ColorState()
    : m_color(1, 1, 1)
    , m_primaryChannelIndex(0)
    , m_model(ColorModel::Rgb)
    , m_converter(new RgbConverter())
    , m_currentColorSpace(nullptr)
    , m_resourceProvider(nullptr)
{
    // TODO load from settings
}

void ColorState::setColorModel(ColorModel model)
{
    m_model = model;
    switch (model) {
    case ColorModel::Rgb:
        m_converter = ColorConverterSP(new RgbConverter());
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

ColorConverterSP ColorState::converter() const
{
    return m_converter;
}

void ColorState::sendToKrita()
{
    RgbConverter rgbConverter;
    QVector3D color = rgbConverter.fromXyz(m_converter->toXyz(m_color));
    // TODO other color spaces
    m_resourceProvider->setFGColor(KoColor(QColor::fromRgbF(color.x(), color.y(), color.z()), m_currentColorSpace));
}

void ColorState::syncFromKrita()
{
    KoColor color = m_resourceProvider->fgColor();
    QVector<float> valuesUnsorted(color.colorSpace()->channelCount());
    color.colorSpace()->normalisedChannelsValue(color.data(), valuesUnsorted);
    const QList<KoChannelInfo *> channelInfo = color.colorSpace()->channels();
    QVector<float> values(channelInfo.size());
    for (int i = 0; i < values.size(); i++) {
        int location = KoChannelInfo::displayPositionToChannelIndex(i, channelInfo);
        values[location] = valuesUnsorted[i];
    }

    // TODO handle other color spaces
    m_color = QVector3D(values[0], values[1], values[2]);
    setColor(m_color);
}

void ColorState::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_currentColorSpace = canvas->image()->colorSpace();
        m_resourceProvider = canvas->imageView()->resourceProvider();
        connect(m_resourceProvider,
                &KisCanvasResourceProvider::sigFGColorChanged,
                this,
                &ColorState::syncFromKrita,
                Qt::UniqueConnection);
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
}

QVector2D ColorState::secondaryChannelValues() const
{
    Q_ASSERT(m_primaryChannelIndex < 3);

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
    Q_ASSERT(m_primaryChannelIndex < 3);

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
