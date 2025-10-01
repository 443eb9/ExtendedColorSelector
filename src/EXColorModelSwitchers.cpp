#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>

#include "EXColorModel.h"
#include "EXColorModelSwitchers.h"
#include "EXColorState.h"

EXColorModelSwitchers::EXColorModelSwitchers(QWidget *parent)
    : QWidget(parent)
{
    auto layout = new QHBoxLayout();
    auto group = new QButtonGroup();
    group->setExclusive(true);

    for (auto id : ColorModelFactory::AllModels) {
        auto model = ColorModelFactory::fromId(id);
        auto button = new QRadioButton(model->displayName());
        layout->addWidget(button);

        connect(button, &QRadioButton::toggled, [id](bool enabled) {
            if (enabled) {
                EXColorState::instance()->setColorModel(id);
            }
        });

        connect(EXColorState::instance(), &EXColorState::sigColorModelChanged, [button, id](ColorModelId newId) {
            button->setChecked(newId == id);
        });
    }

    setLayout(layout);
}
