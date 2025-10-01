#ifndef EXTENDEDCOLORSELECTORDOCK_H
#define EXTENDEDCOLORSELECTORDOCK_H

#include <QDockWidget>
#include <QObject>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
#include "EXColorModelSwitchers.h"

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
    EXChannelSliders *m_channelValues;
    EXColorModelSwitchers *m_colorModelSwitchers;
};

#endif // EXTENDEDCOLORSELECTORDOCK_H
