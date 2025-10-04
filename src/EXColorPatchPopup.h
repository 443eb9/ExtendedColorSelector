#ifndef EXCOLORPATCHPOPUP_H
#define EXCOLORPATCHPOPUP_H

#include <QColor>
#include <QDialog>
#include <QFrame>

class EXColorPatchPopup : public QDialog
{
    Q_OBJECT

public:
    explicit EXColorPatchPopup(QWidget *parent = nullptr);

    void updateColor();
    void recordColor();
    void popupAt(const QPoint &pos);
    void shutdown();

private:
    QFrame *m_currentColorBox;
    QFrame *m_lastColorBox;
    QColor m_lastColor;
};

#endif
