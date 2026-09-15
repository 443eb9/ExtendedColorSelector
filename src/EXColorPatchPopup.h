#ifndef EXCOLORPATCHPOPUP_H
#define EXCOLORPATCHPOPUP_H

#include <QColor>
#include <QDialog>
#include <QFrame>
#include <QWidget>
#include <qwidget.h>

#include "EXEditable.h"

enum EXColorPatchPopupSide {
    Left = 0,
    Right = 1,
};

class EXColorPatchPopup : public QDialog
{
    Q_OBJECT

public:
    explicit EXColorPatchPopup(QWidget *parent = nullptr);
    ~EXColorPatchPopup() override = default;

    void updateCurrentColor(QColor color);
    void updateLastUsedColor(QColor color);
    void updateLastConfirmedColor(QColor color);
    void connectToWidget(const EXEditableImage *bar, const QWidget *around);
    void setSide(EXColorPatchPopupSide side);

private:
    QFrame *m_currentColorBox;
    QFrame *m_lastUsedColorBox;
    QFrame *m_lastConfirmedColorBox;
    QColor m_lastUsedColor;
    QColor m_lastConfirmedColor;
    EXColorPatchPopupSide m_side;
};

#endif
