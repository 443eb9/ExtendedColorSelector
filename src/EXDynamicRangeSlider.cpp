#include <QVBoxLayout>

#include "EXDynamicRangeSlider.h"

EXDynamicRangeSlider::EXDynamicRangeSlider(QWidget *parent)
    : QWidget(parent)
{
    m_dynamicRangeControl = new KisSliderSpinBox(this);
    m_dynamicRangeControl->setRange(80, 10000);
    m_dynamicRangeControl->setExponentRatio(3.0);
    m_dynamicRangeControl->setSingleStep(1);
    m_dynamicRangeControl->setPageStep(100);
    m_dynamicRangeControl->setSuffix(QStringLiteral("cd/m^2"));
    // m_dynamicRangeControl->setValue(qRound(m_colorState->dynamicRange() * 80.0f));
    connect(m_dynamicRangeControl, qOverload<int>(&KisSliderSpinBox::valueChanged), this, [this](int value) {
        Q_EMIT sigDynamicRangeChanged(value / BaseDynamicRange);
    });
    // connect(m_colorState.data(), &EXColorState::sigDynamicRangeChanged, this, [this](float range) {
    //     if (!control) {
    //         return;
    //     }
    //     int expected = qRound(range * 80.0f);
    //     if (expected != control->value()) {
    //         control->setValue(expected);
    //     }
    // });

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_dynamicRangeControl, 0);
    setLayout(layout);
}

float EXDynamicRangeSlider::dynamicRange() const
{
    return m_dynamicRangeControl->value() / BaseDynamicRange;
}

void EXDynamicRangeSlider::setDynamicRange(float dynamicRange)
{
    m_dynamicRangeControl->setValue(qRound(dynamicRange * BaseDynamicRange));
}
