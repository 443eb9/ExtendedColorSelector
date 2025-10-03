#ifndef EXSETTINGSSTATE_H
#define EXSETTINGSSTATE_H

#include <QObject>
#include <QVector>

#include "EXSettings.h"

class EXSettingsState : public QObject
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

#endif // EXSETTINGSSTATE_H
