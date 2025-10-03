#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>

#include "EXColorModel.h"
#include "EXColorModelSwitchers.h"
#include "EXColorState.h"
#include "EXSettingsState.h"

EXColorModelSwitchers::EXColorModelSwitchers(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout();
    auto group = new QButtonGroup();
    group->setExclusive(true);

    connect(EXSettingsState::instance(),
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

    auto &globalSettings = EXSettingsState::instance()->globalSettings;
    auto &settings = EXSettingsState::instance()->settings;
    auto currentModelId = EXColorState::instance()->colorModel()->id();

    for (auto id : globalSettings.displayOrder) {
        if (!settings[id].enabled) {
            continue;
        }

        auto model = ColorModelFactory::fromId(id);
        auto button = new QRadioButton(model->displayName());
        button->setChecked(currentModelId == id);
        layout()->addWidget(button);

        connect(button, &QRadioButton::toggled, button, [id](bool enabled) {
            if (enabled) {
                EXColorState::instance()->setColorModel(id);
            }
        });

        connect(EXColorState::instance(), &EXColorState::sigColorModelChanged, button, [button, id](ColorModelId newId) {
            button->setChecked(newId == id);
        });
    }
}
