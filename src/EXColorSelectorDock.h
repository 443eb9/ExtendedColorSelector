#ifndef EXTENDEDCOLORSELECTORDOCK_H
#define EXTENDEDCOLORSELECTORDOCK_H

#include <QDockWidget>
#include <QObject>
#include <QPushButton>

#include <KisPopupButton.h>
#include <kis_canvas2.h>
#include <kis_color_space_selector.h>
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
    Q_OBJECT

public:
    EXColorSelectorDock();
    ~EXColorSelectorDock() override = default;

    void setViewManager(KisViewManager *kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void enterEvent(QEvent *event) override;
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

    KisPopupButton *m_colorSpaceSelectorButton;
    KisColorSpaceSelector *m_colorSpaceSelector;
    QPushButton *m_useLayerColorSpaceButton;

public Q_SLOTS:
    void onColorSpaceSelected(const KoColorSpace *colorSpace);
};

#endif // EXTENDEDCOLORSELECTORDOCK_H
