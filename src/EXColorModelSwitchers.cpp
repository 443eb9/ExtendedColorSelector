#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>

#include "EXColorModel.h"
#include "EXColorModelSwitchers.h"

EXColorModelSwitchers::EXColorModelSwitchers(EXColorStateSP colorState,
                                             EXSettingsStateSP settingsState,
                                             QWidget *parent)
    : QWidget(parent)
    , m_colorState(colorState)
    , m_settingsState(settingsState)
{
    auto layout = new QHBoxLayout();
    auto group = new QButtonGroup();
    group->setExclusive(true);

    connect(m_settingsState.data(),
            &EXSettingsState::sigSettingsChanged,
            this,
            &EXColorModelSwitchers::settingsChanged);

    setLayout(layout);
}

void EXColorModelSwitchers::settingsChanged()
{
    while (true) {
        auto widget = layout()->takeAt(0);
        if (!widget) {
            break;
        }
        if (auto w = widget->widget()) {
            w->deleteLater();
        }
    }

    auto &globalSettings = m_settingsState->globalSettings;
    auto &settings = m_settingsState->settings;
    auto currentModelId = m_colorState->colorModel()->id();

    for (auto id : globalSettings.displayOrder) {
        if (!settings[id].enabled) {
            continue;
        }

        auto model = ColorModelFactory::fromId(id);
        auto button = new QRadioButton(model->displayName());
        button->setChecked(currentModelId == id);
        layout()->addWidget(button);

        connect(button, &QRadioButton::toggled, this, [this, id](bool enabled) {
            if (enabled) {
                m_colorState->setColorModel(id);
            }
        });

        connect(m_colorState.data(), &EXColorState::sigColorModelChanged, button, [button, id](ColorModelId newId) {
            button->setChecked(newId == id);
        });
    }
}
