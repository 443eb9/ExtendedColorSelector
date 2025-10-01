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
    const ColorModelId colorModelIds[] = {ColorModelId::Rgb};

    for (auto id : colorModelIds) {
        auto model = ColorModelFactory::toModel(id);
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
