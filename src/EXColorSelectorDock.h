#ifndef EXTENDEDCOLORSELECTORDOCK_H
#define EXTENDEDCOLORSELECTORDOCK_H

#include <QDockWidget>
#include <QObject>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
#include "EXColorModelSwitchers.h"
#include "EXColorPatchPopup.h"
#include "EXColorState.h"
#include "EXPortableColorSelector.h"
#include "EXSettingsDialog.h"
#include "EXSettingsState.h"

class EXColorSelectorDock : public QDockWidget, public KisMainwindowObserver
{
public:
    EXColorSelectorDock();

    void setViewManager(KisViewManager *kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void leaveEvent(QEvent *event) override;

private:
    KisCanvas2 *m_canvas;
    EXChannelPlane *m_plane;
    EXChannelSliders *m_sliders;
    EXColorModelSwitchers *m_colorModelSwitchers;
    EXGlobalSettingsDialog *m_globalSettings;
    EXPerColorModelSettingsDialog *m_settings;
    EXPortableColorSelector *m_portableSelector;
    EXColorPatchPopup *m_colorPatchPopup;

    EXColorStateSP m_colorState;
    EXSettingsStateSP m_settingsState;
};

#endif // EXTENDEDCOLORSELECTORDOCK_H
