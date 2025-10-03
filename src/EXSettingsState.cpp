#include "EXSettingsState.h"

static EXSettingsState *s_instance;
EXSettingsState *EXSettingsState::instance()
{
    if (!s_instance) {
        s_instance = new EXSettingsState();
    }
    return s_instance;
}

EXSettingsState::EXSettingsState()
    : QObject()
{
    globalSettings = EXGlobalSettings();

    for (const auto &colorModelId : globalSettings.displayOrder) {
        settings.append(EXPerColorModelSettings(ColorModelFactory::fromId(colorModelId)->displayName()));
    }
}
