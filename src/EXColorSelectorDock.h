#ifndef EXTENDEDCOLORSELECTORDOCK_H
#define EXTENDEDCOLORSELECTORDOCK_H

#include <QDockWidget>
#include <QObject>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>

#include "EXChannelSlider.h"
#include "EXChannelPlane.h"

class ExtendedColorSelectorDock : public QDockWidget, public KisMainwindowObserver
{
public:
    ExtendedColorSelectorDock();

    void setViewManager(KisViewManager *kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    KisCanvas2 *m_canvas;
    ExtendedChannelPlane *m_plane;
    ExtendedChannelSlider *m_channelValues;
};

#endif // EXTENDEDCOLORSELECTORDOCK_H
