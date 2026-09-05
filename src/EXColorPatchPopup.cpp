#include <QFrame>
#include <QHBoxLayout>
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
    m_lastUsedColorBox = new QFrame(this);
    m_lastConfirmedColorBox = new QFrame(this);

    auto historyColorLayout = new QHBoxLayout();
    historyColorLayout->setContentsMargins(0, 0, 0, 0);
    historyColorLayout->setSpacing(0);
    historyColorLayout->addWidget(m_lastConfirmedColorBox, 1);
    historyColorLayout->addWidget(m_lastUsedColorBox, 1);

    mainLayout->addWidget(m_currentColorBox, 2);
    mainLayout->addLayout(historyColorLayout, 1);
}

void EXColorPatchPopup::updateCurrentColor(QColor color)
{
    m_currentColorBox->setStyleSheet(QString("background-color: %1").arg(color.name()));
}

void EXColorPatchPopup::updateLastConfirmedColor(QColor color)
{
    m_lastConfirmedColor = color;
    m_lastConfirmedColorBox->setStyleSheet(QString("background-color: %1").arg(m_lastConfirmedColor.name()));
}

void EXColorPatchPopup::updateLastUsedColor(QColor color)
{
    m_lastUsedColor = color;
    m_lastUsedColorBox->setStyleSheet(QString("background-color: %1").arg(m_lastUsedColor.name()));
}

void EXColorPatchPopup::connectToWidget(const EXEditableImage *widget)
{
    if (!widget) {
        return;
    }

    connect(widget, &EXEditableImage::sigValueChangeStarted, widget, [this, widget]() {
        move(widget->mapToGlobal(QPoint(0, 0)) - QPoint(width(), 0));
        show();
    });
}
