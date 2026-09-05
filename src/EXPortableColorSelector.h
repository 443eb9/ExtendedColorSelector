#ifndef EXPORTABLECOLORSELECTOR_H
#define EXPORTABLECOLORSELECTOR_H

#include <QAction>
#include <QDialog>

#include <KisViewManager.h>
#include <kis_action.h>
#include <kis_canvas2.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
#include "EXColorModelSwitchers.h"
#include "EXColorPatchPopup.h"
#include "EXColorState.h"
#include "EXDynamicRangeSlider.h"
#include "EXSettingsState.h"

class EXPortableColorSelector : public QDialog
{
    Q_OBJECT

public:
    EXPortableColorSelector(QWidget *parent = nullptr);
    ~EXPortableColorSelector() override = default;
    void setViewManager(KisViewManager *kisview);
    void setCanvas(KisCanvas2 *canvas);
    void toggle();
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

public Q_SLOTS:
    void settingsChanged();

private:
    EXChannelPlane *m_plane;
    EXChannelSlidersGroup *m_sliders;
    EXColorModelSwitchers *m_colorModelSwitchers;
    KisAction *m_toggleAction;
    EXColorPatchPopup *m_colorPatchPopup;
    EXDynamicRangeSlider *m_dynamicRangeSlider;

    EXColorStateSP m_colorState;
    EXSettingsStateSP m_settingsState;

    void updateSliders();
};

#endif
