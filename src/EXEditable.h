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

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    virtual void startEdit(QMouseEvent *event, bool isShift) = 0;
    virtual void edit(QMouseEvent *event) = 0;
    virtual void shift(QMouseEvent *event, QVector2D delta) = 0;

private:
    QPointF m_editStart;
};

#endif // EXEDITABLE_H
