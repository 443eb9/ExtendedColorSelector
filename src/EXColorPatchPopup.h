#ifndef EXCOLORPATCHPOPUP_H
#define EXCOLORPATCHPOPUP_H

#include <QColor>
#include <QDialog>
#include <QFrame>
#include <QWidget>

#include "EXEditable.h"

class EXColorPatchPopup : public QDialog
{
    Q_OBJECT

public:
    explicit EXColorPatchPopup(QWidget *parent = nullptr);
    ~EXColorPatchPopup() override = default;

    void updateColor(QColor color);
    void recordColor(QColor color);
    void connectToWidget(const EXEditableGLImage *widget);

private:
    QFrame *m_currentColorBox;
    QFrame *m_lastColorBox;
    QColor m_lastColor;
};

#endif
