#ifndef EXCOLORMODELSWICTHERS_H
#define EXCOLORMODELSWICTHERS_H

#include <QWidget>

#include "EXColorState.h"
#include "EXSettingsState.h"

class EXColorModelSwitchers : public QWidget
{
    Q_OBJECT

public:
    EXColorModelSwitchers(EXColorStateSP colorState, EXSettingsStateSP settingsState, QWidget *parent);
    void settingsChanged();

private:
    EXColorStateSP m_colorState;
    EXSettingsStateSP m_settingsState;
};

#endif
