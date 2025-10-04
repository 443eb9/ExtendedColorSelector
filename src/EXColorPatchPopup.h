#ifndef EXCOLORPATCHPOPUP_H
#define EXCOLORPATCHPOPUP_H

#include <QColor>
#include <QDialog>
#include <QFrame>

#include "EXColorState.h"

class EXColorPatchPopup : public QDialog
{
    Q_OBJECT

public:
    explicit EXColorPatchPopup(EXColorStateSP colorState, QWidget *parent = nullptr);

    void updateColor();
    void recordColor();
    void popupAt(const QPoint &pos);
    void shutdown();

private:
    QFrame *m_currentColorBox;
    QFrame *m_lastColorBox;
    QColor m_lastColor;
    
    EXColorStateSP m_colorState;
};

#endif
