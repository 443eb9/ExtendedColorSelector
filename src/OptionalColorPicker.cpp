#include <QVBoxLayout>

#include "OptionalColorPicker.h"

OptionalColorPicker::OptionalColorPicker(QWidget *parent, const QString &labelText, const QColor &defaultColor)
    : QDialog(parent)
    , cachedColor(defaultColor)
    , indicator(new QPushButton(this))
    , enableBox(new QCheckBox(labelText, this))
    , colorDialog(new QColorDialog(this))
{
    auto mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    mainLayout->addWidget(enableBox);
    mainLayout->addWidget(indicator);

    indicator->setFocusPolicy(Qt::NoFocus);

    connect(indicator, &QPushButton::clicked, this, [this]() {
        colorDialog->show();
    });

    connect(colorDialog, &QColorDialog::colorSelected, this, [this](const QColor &color) {
        cachedColor = color;
        indicator->setStyleSheet(QString("QPushButton { background-color: %1; border: none; }").arg(color.name()));
    });
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
