#ifndef EXEDITABLE_H
#define EXEDITABLE_H

#include <QMouseEvent>
#include <QPointF>
#include <QWidget>

class EXEditable : public QWidget
{
    Q_OBJECT

public:
    EXEditable(QWidget *parent = nullptr);
    ~EXEditable() override = default;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void startEdit(QMouseEvent *event, bool isShift) = 0;
    virtual void edit(QMouseEvent *event) = 0;
    virtual void shift(QMouseEvent *event, QVector2D delta) = 0;

Q_SIGNALS:
    void sigValueChangeStarted();
    void sigValueChanging();
    void sigValueFinalized();

private:
    QPointF m_editStart;
};

#endif // EXEDITABLE_H
