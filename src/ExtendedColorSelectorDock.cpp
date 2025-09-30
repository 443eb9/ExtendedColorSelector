#include <QVBoxLayout>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas_resource_provider.h>
#include <kis_display_color_converter.h>

#include "ExtendedColorSelectorDock.h"

ExtendedColorSelectorDock::ExtendedColorSelectorDock()
    : QDockWidget("Extended Color Selector")
{
    m_canvas = nullptr;
    auto mainLayout = new QVBoxLayout();

    m_plane = new SecondaryChannelsPlane(this);
    mainLayout->addWidget(m_plane);
    m_bar = new PrimaryChannelBar(this);
    mainLayout->addWidget(m_bar);

    auto mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);
}

void ExtendedColorSelectorDock::setViewManager(KisViewManager *kisview)
{
}

void ExtendedColorSelectorDock::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = qobject_cast<KisCanvas2 *>(canvas);
    if (m_canvas) {
        ColorState::instance()->setCanvas(m_canvas);
        m_plane->setCanvas(m_canvas);
        m_bar->setCanvas(m_canvas);
    }
}

void ExtendedColorSelectorDock::unsetCanvas()
{
    m_canvas = nullptr;
    m_plane->setCanvas(nullptr);
}
