#include <QtMath>

#include <kis_canvas2.h>
#include <kis_display_color_converter.h>
#include <opengl/KisOpenGLModeProber.h>

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
    , m_dri(nullptr)
    , m_dcc(nullptr)
    , m_colorConverter(nullptr)
    , m_blockColorSync(false)
    , m_useLayerColorSpace(false)
    , m_dynamicRange(1.0f)
    , m_hasHardwareHDR(KisOpenGLModeProber::instance()->useHDRMode())
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

    m_color = m_colorModel->transferTo(newModel, m_color, m_color);
    ExtendedUtils::saturateColor(m_color);
    m_colorModel = newModel;
    m_colorConverter = new EXKoColorConverter(m_currentColorSpace);

    Q_EMIT sigColorModelChanged(model);
    Q_EMIT sigColorChanged(m_color);
}

const ColorModelSP EXColorState::colorModel() const
{
    return m_colorModel;
}

void EXColorState::sendToKrita()
{
    QVector3D currentColor = m_colorModel->transferTo(kritaColorModel(), m_color);
    ExtendedUtils::saturateColor(currentColor);
    currentColor *= dynamicRange();

    m_blockColorSync = true;
    m_resourceProvider->setFGColor(m_colorConverter->displayChannelsToKoColor(QVector4D(currentColor, 1.0f)));
    m_blockColorSync = false;
}

void EXColorState::syncFromKrita()
{
    if (m_blockColorSync || !m_resourceProvider || !m_currentColorSpace || !m_colorModel) {
        return;
    }

    KoColor koColor = m_resourceProvider->fgColor();
    koColor.convertTo(m_currentColorSpace);
    QVector3D newColor = m_colorConverter->koColorToDisplayChannels(koColor).toVector3D();
    m_color = kritaColorModel()->transferTo(m_colorModel, newColor, m_color);
    setColor(m_color);
}

void EXColorState::setCanvas(KisCanvas2 *canvas)
{
    if (canvas) {
        m_resourceProvider = canvas->imageView()->resourceProvider();
        m_dcc = canvas->displayColorConverter();
        m_dri = canvas->displayColorConverter()->displayRendererInterface();

        connect(m_resourceProvider, &KisCanvasResourceProvider::sigFGColorChanged, this, &EXColorState::syncFromKrita);

        if (m_useLayerColorSpace) {
            setColorSpace(m_dcc->paintingColorSpace());
            connect(m_dcc,
                    &KisDisplayColorConverter::displayConfigurationChanged,
                    this,
                    &EXColorState::onDisplayConfigChanged,
                    Qt::UniqueConnection);
        } else {
            syncFromKrita();
        }
    }
}

void EXColorState::setPrimaryChannelValue(float value)
{
    m_color[m_primaryChannelIndex] = value;
    setColor(m_color);
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

void EXColorState::setSecondaryChannelValues(const QVector2D &values)
{
    switch (m_colorModel->channelCount()) {
    case 2:
        m_color = values.toVector3D();
        break;
    case 3:
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
        break;
    }

    setColor(m_color);
}

QVector3D EXColorState::color() const
{
    return m_color;
}

KoColor EXColorState::koColor() const
{
    auto kritaColor = m_colorModel->transferTo(kritaColorModel(), m_color);
    kritaColor *= dynamicRange();
    return m_colorConverter->displayChannelsToKoColor(QVector4D(kritaColor, 1.0f));
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
    return m_colorConverter->colorModel();
}

const EXColorConverterSP EXColorState::koColorConverter() const
{
    return m_colorConverter;
}

void EXColorState::setDynamicRange(float dynamicRange)
{
    float clampedRange = qMax(0.0f, dynamicRange);
    if (qAbs(m_dynamicRange - clampedRange) < 1e-6f) {
        return;
    }

    m_dynamicRange = clampedRange;
    Q_EMIT sigDynamicRangeChanged(m_dynamicRange);

    if (m_resourceProvider) {
        sendToKrita();
    }
}

void EXColorState::setUseLayerColorSpace(bool use)
{
    m_useLayerColorSpace = use;
    if (!m_dcc) {
        return;
    }

    if (use) {
        setColorSpace(m_dcc->paintingColorSpace());
        connect(m_dcc,
                &KisDisplayColorConverter::displayConfigurationChanged,
                this,
                &EXColorState::onDisplayConfigChanged,
                Qt::UniqueConnection);
    } else {
        disconnect(m_dcc, nullptr, this, nullptr);
    }
}

void EXColorState::setColorSpace(const KoColorSpace *colorSpace)
{
    m_currentColorSpace = colorSpace;
    m_colorConverter = new EXKoColorConverter(colorSpace);

    if (EXSettingsState::instance()->globalSettings.alwaysUseSrgbModelForHsvAndHsl
        || !colorSpace->profile()->isLinear()) {
        SRGBModel::IntermediateModelForHsvAndHsl = ColorModelFactory::fromId(ColorModelId::Srgb);
    } else {
        SRGBModel::IntermediateModelForHsvAndHsl = ColorModelFactory::fromId(ColorModelId::LinearRgb);
    }

    syncFromKrita();
    Q_EMIT sigColorSpaceChanged(m_currentColorSpace);
}

void EXColorState::onDisplayConfigChanged()
{
    if (m_useLayerColorSpace && m_dcc) {
        setColorSpace(m_dcc->paintingColorSpace());
    }
}

float EXColorState::dynamicRange() const
{
    return hdrSupported() ? m_dynamicRange : 1.0f;
}

bool EXColorState::hdrSupported() const
{
    return m_hasHardwareHDR && m_colorConverter && m_colorConverter->colorSpace()->hasHighDynamicRange();
}

void EXColorState::connectChannelPlane(EXChannelPlane *plane)
{
    // Assume the color model inside the plane is always the same as we use here.
    plane->setColorModel(m_colorModel);
    plane->setColor(m_color, m_colorModel);
    plane->setColorConverter(m_colorConverter);
    plane->setDynamicRange(dynamicRange());
    plane->setUseHdr(hdrSupported());
    plane->updateImage();

    connect(plane, &EXChannelPlane::sigPrimaryChannelValueSelected, this, &EXColorState::setPrimaryChannelValue);
    connect(plane, &EXChannelPlane::sigSecondaryChannelsValueSelected, this, &EXColorState::setSecondaryChannelValues);
    connect(plane, &EXChannelPlane::sigValueFinalized, this, &EXColorState::sendToKrita);
    connect(this, &EXColorState::sigColorChanged, plane, [this, plane](QVector3D color) {
        plane->setColor(color, m_colorModel);
    });
    connect(this, &EXColorState::sigColorModelChanged, plane, [this, plane]() {
        plane->setColorModel(m_colorModel);
        plane->setColor(m_color, m_colorModel);
        plane->updateImage();
    });
    connect(this, &EXColorState::sigColorSpaceChanged, plane, [this, plane](const KoColorSpace *) {
        plane->setColorConverter(m_colorConverter);
        plane->setDynamicRange(dynamicRange());
        plane->setUseHdr(hdrSupported());
        plane->updateImage();
    });
    connect(this, &EXColorState::sigPrimaryChannelIndexChanged, plane, [plane](quint32 index) {
        plane->setPrimaryChannelIndex(index);
        plane->updateImage();
    });
    connect(this, &EXColorState::sigDynamicRangeChanged, plane, [plane](float dynamicRange) {
        plane->setDynamicRange(dynamicRange);
        plane->updateImage();
    });
}

void EXColorState::connectChannelSlider(EXChannelSlider *slider)
{
    auto result = slider->colorModelAndChannelIndex();
    auto colorModel = result.first;
    auto channelIndex = result.second;
    slider->setColorConverter(m_colorConverter);
    slider->setColor(m_color, m_colorModel);
    slider->setActive(colorModel->id() == m_colorModel->id());
    slider->setSelected(channelIndex == m_primaryChannelIndex);
    slider->setDynamicRange(dynamicRange());
    slider->setUseHdr(hdrSupported());
    slider->updateImage();

    connect(slider->bar(), &EXChannelSliderBar::sigValueChanging, this, [this, colorModel, slider]() {
        setColor(colorModel->transferTo(m_colorModel.data(), slider->colorAtCurrentModel(), m_color));
    });
    connect(slider->bar(), &EXChannelSliderBar::sigValueFinalized, this, &EXColorState::sendToKrita);
    connect(slider, &EXChannelSlider::sigSelected, this, [this, channelIndex]() {
        setPrimaryChannelIndex(channelIndex);
    });

    connect(this, &EXColorState::sigColorChanged, slider, [this, slider](QVector3D color) {
        slider->setColor(color, m_colorModel);
        slider->updateImage();
    });
    connect(this, &EXColorState::sigColorModelChanged, slider, [this, colorModel, slider](ColorModelId modelId) {
        slider->setColor(m_color, m_colorModel);
        slider->setActive(colorModel->id() == modelId);
        slider->setUseHdr(hdrSupported());
        slider->setDynamicRange(dynamicRange());
        slider->updateImage();
    });
    connect(this, &EXColorState::sigColorSpaceChanged, slider, [this, slider](const KoColorSpace *) {
        slider->setColorConverter(m_colorConverter);
        slider->setDynamicRange(dynamicRange());
        slider->setUseHdr(hdrSupported());
        slider->updateImage();
    });
    connect(this, &EXColorState::sigPrimaryChannelIndexChanged, slider, [channelIndex, slider](quint32 index) {
        slider->setSelected(channelIndex == index);
        slider->updateImage();
    });
    connect(this, &EXColorState::sigDynamicRangeChanged, slider, [slider](float dynamicRange) {
        slider->setDynamicRange(dynamicRange);
        slider->updateImage();
    });
}

void EXColorState::connectDynamicRangeSlider(EXDynamicRangeSlider *slider)
{
    slider->setDynamicRange(m_dynamicRange);
    slider->setVisible(hdrSupported());

    connect(slider, &EXDynamicRangeSlider::sigDynamicRangeChanged, this, &EXColorState::setDynamicRange);
    connect(this, &EXColorState::sigDynamicRangeChanged, slider, &EXDynamicRangeSlider::setDynamicRange);
    connect(this, &EXColorState::sigColorSpaceChanged, slider, [this, slider](const KoColorSpace *) {
        slider->setVisible(hdrSupported());
    });
}
