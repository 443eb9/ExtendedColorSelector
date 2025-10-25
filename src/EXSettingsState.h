#ifndef EXSETTINGSSTATE_H
#define EXSETTINGSSTATE_H

#include <QObject>
#include <QVector>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
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
    void connectChannelSlider(EXChannelSlider *slider);

    void applySettingsToPlane(EXChannelPlane *plane);
    void applySettingsToSlider(EXChannelSlider *slider);

Q_SIGNALS:
    void sigSettingsChanged();
};

typedef KisSharedPtr<EXSettingsState> EXSettingsStateSP;

#endif // EXSETTINGSSTATE_H
