#ifndef EXPORTABLECOLORSELECTOR_H
#define EXPORTABLECOLORSELECTOR_H

#include <QAction>
#include <QDialog>

#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_action.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
#include "EXColorModelSwitchers.h"

class EXPortableColorSelector : public QDialog
{
    Q_OBJECT

public:
    EXPortableColorSelector(QWidget *parent = nullptr);
    void setViewManager(KisViewManager *kisview);
    void setCanvas(KisCanvas2 *canvas);
    void toggle();
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public Q_SLOTS:
    void settingsChanged();

private:
    EXChannelPlane *m_plane;
    EXChannelSliders *m_sliders;
    EXColorModelSwitchers *m_colorModelSwitchers;
    KisAction *m_toggleAction;
};

#endif
