#include <QHBoxLayout>

#include "OptionalColorPicker.h"

OptionalColorPicker::OptionalColorPicker(QWidget *parent, const QString &labelText, const QColor &defaultColor)
    : QWidget(parent)
    , cachedColor(defaultColor)
    , indicator(new QPushButton(this))
    , enableBox(new QCheckBox(labelText, this))
    , colorDialog(new QColorDialog(this))
{
    auto mainLayout = new QHBoxLayout(this);
    setLayout(mainLayout);

    mainLayout->addWidget(enableBox);
    mainLayout->addWidget(indicator);

    indicator->setFocusPolicy(Qt::NoFocus);

    connect(indicator, &QPushButton::clicked, this, [this]() {
        colorDialog->exec();
    });

    auto updateColor = [this](const QColor &color) {
        cachedColor = color;
        indicator->setStyleSheet(QString("QPushButton { background-color: %1; border: none; }").arg(color.name()));
    };
    updateColor(defaultColor);
    connect(colorDialog, &QColorDialog::colorSelected, this, updateColor);
}

void OptionalColorPicker::setPickingEnabled(bool enabled)
{
    if (enabled) {
        indicator->show();
    } else {
        indicator->hide();
    }
    enableBox->setChecked(enabled);
}
