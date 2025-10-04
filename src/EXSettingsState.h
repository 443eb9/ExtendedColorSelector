#ifndef EXSETTINGSSTATE_H
#define EXSETTINGSSTATE_H

#include <QObject>
#include <QVector>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "EXSettings.h"

class EXSettingsState : public QObject, public KisShared
{
    Q_OBJECT

public:
    static EXSettingsState *instance();

    EXSettingsState();

    EXGlobalSettings globalSettings;
    QVector<EXPerColorModelSettings> settings;

Q_SIGNALS:
    void sigSettingsChanged();
};

typedef KisSharedPtr<EXSettingsState> EXSettingsStateSP;

#endif // EXSETTINGSSTATE_H
