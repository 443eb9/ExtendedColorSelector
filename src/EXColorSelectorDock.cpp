#include <QVBoxLayout>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "EXColorSelectorDock.h"
#include "EXColorState.h"

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
    }
}

void EXColorSelectorDock::unsetCanvas()
{
    m_canvas = nullptr;
    m_plane->setCanvas(nullptr);
}
