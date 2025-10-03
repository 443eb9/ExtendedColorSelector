#include <QPushButton>
#include <QVBoxLayout>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>
#include <kis_icon_utils.h>

#include "EXColorSelectorDock.h"
#include "EXColorState.h"
#include "EXSettingsState.h"

EXColorSelectorDock::EXColorSelectorDock()
    : QDockWidget("Extended Color Selector")
{
    m_canvas = nullptr;
    auto mainLayout = new QVBoxLayout();

    m_plane = new EXChannelPlane(this);
    mainLayout->addWidget(m_plane);
    m_colorModelSwitchers = new EXColorModelSwitchers(this);
    mainLayout->addWidget(m_colorModelSwitchers);
    m_channelValues = new EXChannelSliders(this);
    mainLayout->addWidget(m_channelValues);
    m_settings = new EXPerColorModelSettingsDialog(this);
    m_globalSettings = new EXGlobalSettingsDialog(this);

    auto settingsButtonLayout = new QHBoxLayout(this);
    auto settingsButton = new QPushButton();
    settingsButton->setIcon(KisIconUtils::loadIcon(("configure")));
    settingsButton->setFlat(true);
    connect(settingsButton, &QPushButton::clicked, this, [this]() {
        m_settings->show();
    });
    auto globalSettingsButton = new QPushButton(this);
    globalSettingsButton->setIcon(KisIconUtils::loadIcon(("applications-system")));
    globalSettingsButton->setFlat(true);
    connect(globalSettingsButton, &QPushButton::clicked, this, [this]() {
        m_globalSettings->show();
    });
    settingsButtonLayout->addWidget(settingsButton);
    settingsButtonLayout->addStretch(1);
    settingsButtonLayout->addWidget(globalSettingsButton);
    mainLayout->addLayout(settingsButtonLayout);

    auto mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);
}

void EXColorSelectorDock::setViewManager(KisViewManager *kisview)
{
}

void EXColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = qobject_cast<KisCanvas2 *>(canvas);
    if (m_canvas) {
        m_plane->setCanvas(m_canvas);
        m_channelValues->setCanvas(m_canvas);
        // This emits signals that requires the canvas to be set.
        // So color state should be set canvas after other widgets.
        EXColorState::instance()->setCanvas(m_canvas);
        Q_EMIT EXSettingsState::instance()->sigSettingsChanged();
    }
}

void EXColorSelectorDock::unsetCanvas()
{
    m_canvas = nullptr;
    m_plane->setCanvas(nullptr);
}
