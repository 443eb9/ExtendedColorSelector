#include <QVBoxLayout>

#include <KisViewManager.h>
#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>
#include <kis_icon_utils.h>

#include "EXColorModel.h"
#include "EXColorSelectorDock.h"

EXColorSelectorDock::EXColorSelectorDock()
    : QDockWidget("Extended Color Selector")
    , m_canvas(nullptr)
    , m_colorState(EXColorState::instance())
    , m_settingsState(EXSettingsState::instance())
{
    m_canvas = nullptr;
    auto mainLayout = new QVBoxLayout();

    m_colorPatchPopup = new EXColorPatchPopup(this);
    connect(m_colorState.data(), &EXColorState::sigColorChanged, this, [this]() {
        m_colorPatchPopup->updateColor(m_colorState->qColor());
    });

    auto colorSpaceLayout = new QHBoxLayout(this);
    m_colorSpaceSelectorButton = new KisPopupButton(this);
    m_colorSpaceSelector = new KisColorSpaceSelector(this);
    m_colorSpaceSelector->showColorBrowserButton(false);
    m_useLayerColorSpaceButton = new QPushButton(this);
    m_useLayerColorSpaceButton->setCheckable(true);
    m_colorSpaceSelectorButton->setPopupWidget(m_colorSpaceSelector);
    m_colorSpaceSelectorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    colorSpaceLayout->addWidget(m_colorSpaceSelectorButton);
    colorSpaceLayout->addWidget(m_useLayerColorSpaceButton);
    mainLayout->addLayout(colorSpaceLayout);

    connect(m_colorState.data(), &EXColorState::sigColorSpaceChanged, this, [this](const KoColorSpace *colorSpace) {
        if (colorSpace != m_colorSpaceSelector->currentColorSpace()) {
            m_colorSpaceSelector->setCurrentColorSpace(colorSpace);
        }
        m_colorSpaceSelectorButton->setText(colorSpace->name());
    });
    connect(m_useLayerColorSpaceButton, &QPushButton::toggled, this, [this](bool checked) {
        auto &settings = m_settingsState->globalSettings;
        settings.useLayerColorSpace = checked;
        m_colorState->setUseLayerColorSpace(checked);
        m_colorSpaceSelectorButton->setEnabled(!checked);
        settings.customColorSpace = m_colorSpaceSelector->currentColorSpace();
        m_useLayerColorSpaceButton->setIcon(checked ? KisIconUtils::loadIcon("chain-icon")
                                                    : KisIconUtils::loadIcon("chain-broken-icon"));
        settings.writeAll();
    });
    connect(m_colorSpaceSelector,
            SIGNAL(colorSpaceChanged(const KoColorSpace *)),
            this,
            SLOT(onColorSpaceSelected(const KoColorSpace *)));

    m_plane = new EXChannelPlane(this);
    m_plane->setColorModel(ColorModelFactory::fromId((ColorModelId)m_settingsState->globalSettings.currentColorModel));
    m_colorState->connectChannelPlane(m_plane);
    m_settingsState->connectChannelPlane(m_plane);
    m_settingsState->updateConnectedChannelPlanes();
    connect(m_plane, &EXChannelPlane::sigStartColorSelection, this, [this]() {
        m_colorPatchPopup->popupAtWidget(m_plane);
    });
    connect(m_plane, &EXChannelPlane::sigValuesFinalized, this, [this]() {
        if (m_settingsState->globalSettings.recordLastColorWhenMouseRelease) {
            m_colorPatchPopup->recordColor(m_colorState->qColor());
        }
    });

    m_colorModelSwitchers = new EXColorModelSwitchers(m_colorState, m_settingsState, this);
    m_sliders =
        new EXChannelSlidersGroup(QVector<ColorModelId>(), m_colorState, m_settingsState, m_colorPatchPopup, this);
    mainLayout->addWidget(m_plane);
    mainLayout->addWidget(m_colorModelSwitchers);
    mainLayout->addWidget(m_sliders);
    mainLayout->addStretch(1);

    updateSliders();
    connect(m_colorState.data(), &EXColorState::sigColorModelChanged, this, &EXColorSelectorDock::updateSliders);
    connect(m_settingsState.data(), &EXSettingsState::sigSettingsChanged, this, &EXColorSelectorDock::updateSliders);

    m_settings = new EXPerColorModelSettingsDialog(m_settingsState, this);
    m_globalSettings = new EXGlobalSettingsDialog(m_settingsState, this);

    auto settingsButtonLayout = new QHBoxLayout(this);
    auto settingsButton = new QPushButton();
    settingsButton->setIcon(KisIconUtils::loadIcon(("configure")));
    settingsButton->setFlat(true);
    connect(settingsButton, &QPushButton::clicked, this, [this]() {
        m_settings->exec();
    });
    auto globalSettingsButton = new QPushButton(this);
    globalSettingsButton->setIcon(KisIconUtils::loadIcon(("applications-system")));
    globalSettingsButton->setFlat(true);
    connect(globalSettingsButton, &QPushButton::clicked, this, [this]() {
        m_globalSettings->exec();
    });
    settingsButtonLayout->addWidget(settingsButton);
    settingsButtonLayout->addStretch(1);
    settingsButtonLayout->addWidget(globalSettingsButton);
    mainLayout->addLayout(settingsButtonLayout);

    auto mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);

    m_portableSelector = new EXPortableColorSelector();

    m_useLayerColorSpaceButton->setChecked(m_settingsState->globalSettings.useLayerColorSpace);
    m_colorState->setUseLayerColorSpace(m_settingsState->globalSettings.useLayerColorSpace);
    if (m_settingsState->globalSettings.useLayerColorSpace) {
        m_useLayerColorSpaceButton->setIcon(KisIconUtils::loadIcon("chain-icon"));
    } else {
        m_useLayerColorSpaceButton->setIcon(KisIconUtils::loadIcon("chain-broken-icon"));
        auto customColorSpace = m_settingsState->globalSettings.customColorSpace;
        m_colorSpaceSelector->setCurrentColorSpace(customColorSpace);
        m_colorState->setColorSpace(customColorSpace);
    }

    connect(m_colorState.data(), &EXColorState::sigColorModelChanged, this, [this]() {
        m_settingsState->updateConnectedChannelPlanes();
    });
}

void EXColorSelectorDock::setViewManager(KisViewManager *kisview)
{
    m_portableSelector->setViewManager(kisview);
}

void EXColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = qobject_cast<KisCanvas2 *>(canvas);
    if (m_canvas) {
        m_plane->setCanvas(m_canvas);
        m_sliders->setCanvas(m_canvas);
        m_portableSelector->setCanvas(m_canvas);
        m_colorState->setCanvas(m_canvas);
        Q_EMIT m_settingsState->sigSettingsChanged();
    }
}

void EXColorSelectorDock::unsetCanvas()
{
    m_canvas = nullptr;
    m_plane->setCanvas(nullptr);
    m_sliders->setCanvas(nullptr);
    m_colorState->setCanvas(nullptr);
    m_portableSelector->setCanvas(nullptr);
}

void EXColorSelectorDock::enterEvent(QEvent *event)
{
    QDockWidget::enterEvent(event);
    m_colorPatchPopup->recordColor(m_colorState->qColor());
}

void EXColorSelectorDock::leaveEvent(QEvent *event)
{
    QDockWidget::leaveEvent(event);
    m_colorPatchPopup->hide();
}

void EXColorSelectorDock::onColorSpaceSelected(const KoColorSpace *colorSpace)
{
    auto &settings = m_settingsState->globalSettings;
    if (!settings.useLayerColorSpace) {
        m_colorState->setColorSpace(colorSpace);
        settings.customColorSpace = colorSpace;
        settings.writeAll();
    }
}

void EXColorSelectorDock::updateSliders()
{
    auto &settings = m_settingsState->settings[m_colorState->colorModel()->id()];
    if (settings.slidersEnabled) {
        auto sliders = QVector(settings.extraSliders);
        sliders.prepend(m_colorState->colorModel()->id());
        m_sliders->resetColorModels(sliders);
    } else {
        m_sliders->resetColorModels(settings.extraSliders);
    }
}
