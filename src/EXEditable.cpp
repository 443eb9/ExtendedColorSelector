#include "EXEditable.h"

EXEditable::EXEditable(QWidget *parent)
    : QWidget(parent)
{
}

void EXEditable::mousePressEvent(QMouseEvent *event)
{
    m_editStart = event->pos();
}

void EXEditable::mouseMoveEvent(QMouseEvent *event)
{
    auto modifiers = event->modifiers();
    float factor = 1.0;
    if (modifiers.testFlag(Qt::ShiftModifier)) {
        factor = 0.1;
    } else if (modifiers.testFlag(Qt::AltModifier)) {
        factor = 0.01;
    }

    if (factor == 1.0f) {
        edit(event);
    } else {
        shift(event, QVector2D(event->pos() - m_editStart) * factor);
    }
}
