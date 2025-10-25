#ifndef EXDYNAMICRANGESLIDER_H
#define EXDYNAMICRANGESLIDER_H

#include <QWidget>

#include <kis_slider_spin_box.h>

class EXDynamicRangeSlider : public QWidget
{
    Q_OBJECT

public:
    explicit EXDynamicRangeSlider(QWidget *parent = nullptr);
    ~EXDynamicRangeSlider() override = default;

    float dynamicRange() const;
    void setDynamicRange(float dynamicRange);

    const float BaseDynamicRange = 80.0f;

Q_SIGNALS:
    void sigDynamicRangeChanged(float dynamicRange);

private:
    KisSliderSpinBox *m_dynamicRangeControl;
};

#endif // EXDYNAMICRANGESLIDER_H
