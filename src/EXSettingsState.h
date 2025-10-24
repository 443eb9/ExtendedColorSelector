#ifndef EXSETTINGSSTATE_H
#define EXSETTINGSSTATE_H

#include <QObject>
#include <QVector>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "EXChannelPlane.h"
#include "EXSettings.h"

class EXSettingsState : public QObject, public KisShared
{
    Q_OBJECT

public:
    static EXSettingsState *instance();

    EXSettingsState();
    ~EXSettingsState() override = default;

    EXGlobalSettings globalSettings;
    QVector<EXPerColorModelSettings> settings;

    void connectChannelPlane(EXChannelPlane *plane);
    void updateConnectedChannelPlanes();

Q_SIGNALS:
    void sigSettingsChanged();

private:
    QVector<EXChannelPlane *> m_connectedChannelPlanes;
};

typedef KisSharedPtr<EXSettingsState> EXSettingsStateSP;

#endif // EXSETTINGSSTATE_H
