#include <kis_canvas2.h>
#include <kis_display_color_converter.h>

#include "EXColorState.h"
#include "EXSettingsState.h"
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
    , m_colorModel(
          ColorModelFactory::fromId((ColorModelId)EXSettingsState::instance()->globalSettings.currentColorModel))
    , m_currentColorSpace(nullptr)
    , m_resourceProvider(nullptr)
    , m_koColorConverter(nullptr)
    , m_blockSync(false)
{
}

void EXColorState::setColorModel(ColorModelId model)
{
    if (m_colorModel->id() == model) {
        return;
    }

    auto &settings = EXSettingsState::instance()->globalSettings;
    settings.currentColorModel = model;
    settings.writeAll();

    m_primaryChannelIndex = EXSettingsState::instance()->settings[model].primaryIndex;
    Q_EMIT sigPrimaryChannelIndexChanged(m_primaryChannelIndex);

    auto newModel = ColorModelFactory::fromId(model);

    m_color = m_colorModel->transferTo(newModel, m_color, nullptr);
    ExtendedUtils::saturateColor(m_color);
    m_colorModel = newModel;
    m_koColorConverter = new EXColorConverter(m_currentColorSpace, m_colorModel);
    Q_EMIT sigColorModelChanged(model);
    Q_EMIT sigColorChanged(m_color);
}

const ColorModelSP EXColorState::colorModel() const
{
    return m_colorModel;
}

void EXColorState::sendToKrita()
{
    QVector3D currentColor = m_colorModel->transferTo(m_kritaColorModel, m_color, nullptr);
    ExtendedUtils::saturateColor(currentColor);

    m_blockSync = true;
    m_resourceProvider->setFGColor(m_koColorConverter->displayChannelsToKoColor(QVector4D(currentColor, 1.0f)));
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
            setColorSpace(canvas->image()->colorSpace());
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

    auto &settings = EXSettingsState::instance()->settings[m_colorModel->id()];
    settings.primaryIndex = index;
    settings.writeAll();

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
    return m_koColorConverter->displayChannelsToKoColor(QVector4D(kritaColor, 1.0f));
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

bool EXColorState::possibleOutOfSrgb() const
{
    return m_kritaColorModel->isSrgbBased() && !m_colorModel->isSrgbBased();
}

const EXColorConverterSP EXColorState::koColorConverter() const
{
    return m_koColorConverter;
}

void EXColorState::setColorSpace(const KoColorSpace *colorSpace)
{
    m_currentColorSpace = colorSpace;
    m_koColorConverter = new EXColorConverter(colorSpace, m_colorModel);
    m_kritaColorModel = ColorModelFactory::fromKoColorSpace(colorSpace);
}
