#include <QFrame>
#include <QVBoxLayout>

#include "EXColorPatchPopup.h"
#include "EXColorState.h"

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

    connect(EXColorState::instance(), &EXColorState::sigColorChanged, this, &EXColorPatchPopup::updateColor);
}

void EXColorPatchPopup::updateColor()
{
    auto currentColor = EXColorState::instance()->qColor();
    m_currentColorBox->setStyleSheet(QString("background-color: %1").arg(currentColor.name()));
}

void EXColorPatchPopup::recordColor()
{
    m_lastColor = EXColorState::instance()->qColor();
    m_lastColorBox->setStyleSheet(QString("background-color: %1").arg(m_lastColor.name()));
}

void EXColorPatchPopup::popupAt(const QPoint &pos)
{
    recordColor();
    move(pos);
    show();
}

void EXColorPatchPopup::shutdown()
{
    m_lastColor = EXColorState::instance()->qColor();
    hide();
}
