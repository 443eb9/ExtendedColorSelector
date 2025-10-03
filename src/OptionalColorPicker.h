#ifndef OPTIONALCOLORPICKER_H
#define OPTIONALCOLORPICKER_H

#include <QCheckBox>
#include <QColorDialog>
#include <QPushButton>
#include <QWidget>

class OptionalColorPicker : public QWidget
{
    Q_OBJECT

public:
    OptionalColorPicker(QWidget *parent, const QString &labelText, const QColor &defaultColor);
    void setPickingEnabled(bool enabled);

    QColorDialog *colorDialog;
    QColor cachedColor;
    QCheckBox *enableBox;
    QPushButton *indicator;
};

#endif // OPTIONALCOLORPICKER_H
