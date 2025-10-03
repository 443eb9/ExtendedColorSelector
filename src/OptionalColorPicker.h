#ifndef OPTIONALCOLORPICKER_H
#define OPTIONALCOLORPICKER_H

#include <QCheckBox>
#include <QColorDialog>
#include <QDialog>
#include <QPushButton>

class OptionalColorPicker : public QDialog
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
