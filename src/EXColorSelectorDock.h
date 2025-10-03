#ifndef EXTENDEDCOLORSELECTORDOCK_H
#define EXTENDEDCOLORSELECTORDOCK_H

#include <QDockWidget>
#include <QObject>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
#include "EXColorModelSwitchers.h"
#include "EXSettingsDialog.h"
#include "EXPortableColorSelector.h"

class EXColorSelectorDock : public QDockWidget, public KisMainwindowObserver
{
public:
    EXColorSelectorDock();

    void setViewManager(KisViewManager *kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    KisCanvas2 *m_canvas;
    EXChannelPlane *m_plane;
    EXChannelSliders *m_sliders;
    EXColorModelSwitchers *m_colorModelSwitchers;
    EXGlobalSettingsDialog* m_globalSettings;
    EXPerColorModelSettingsDialog* m_settings;
    EXPortableColorSelector* m_portableSelector;
};

#endif // EXTENDEDCOLORSELECTORDOCK_H
