#include <QFrame>
#include <QVBoxLayout>

#include "EXColorPatchPopup.h"

EXColorPatchPopup::EXColorPatchPopup(QWidget *parent)
    : QDialog(parent)
{
    auto mainLayout = new QVBoxLayout(this);
    setFixedSize(100, 150);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setWindowFlag(Qt::WindowType::FramelessWindowHint, true);
    setWindowFlag(Qt::WindowType::Tool, true);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    m_currentColorBox = new QFrame(this);
    m_lastColorBox = new QFrame(this);

    mainLayout->addWidget(m_currentColorBox, 1);
    mainLayout->addWidget(m_lastColorBox, 1);
}

void EXColorPatchPopup::updateColor(QColor color)
{
    m_currentColorBox->setStyleSheet(QString("background-color: %1").arg(color.name()));
}

void EXColorPatchPopup::recordColor(QColor color)
{
    m_lastColor = color;
    m_lastColorBox->setStyleSheet(QString("background-color: %1").arg(m_lastColor.name()));
}

void EXColorPatchPopup::connectToWidget(const EXEditableGLImage *widget)
{
    if (!widget) {
        return;
    }

    connect(widget, &EXEditableGLImage::sigValueChangeStarted, widget, [this, widget]() {
        move(widget->mapToGlobal(QPoint(0, 0)) - QPoint(width(), 0));
        show();
    });
}
