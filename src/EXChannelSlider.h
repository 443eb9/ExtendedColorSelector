#ifndef ExtendedChannelSlider_H
#define ExtendedChannelSlider_H

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPair>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QVector3D>
#include <QVector4D>
#include <QVector>
#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas2.h>
#include <kis_display_color_converter.h>

#include "EXColorModel.h"
#include "EXEditable.h"
#include "EXKoColorConverter.h"

class EXChannelSliderBar : public EXEditableImage
{
    Q_OBJECT

    friend class EXChannelSlider;

public:
    EXChannelSliderBar(int channelIndex, ColorModelSP colorModel, QWidget *parent = nullptr);
    ~EXChannelSliderBar() override = default;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

    void startEdit(QMouseEvent *event, bool isShift) override;
    void edit(QMouseEvent *event) override;
    void shift(QMouseEvent *event, QVector2D delta) override;

    float currentWidgetCoord();

    void setCanvas(KisCanvas2 *canvas);

private:
    int m_channelIndex;
    float m_editStart;
    QVector3D m_colorAtCurrentModel;
    EXColorConverterSP m_converter;
    ColorModelSP m_colorModel;
    bool m_colorful;
    bool m_sanitizeOutOfGamut;
    QVector3D m_outOfGamutColor;
    float m_dynamicRange;
    bool m_imageDirty;

    void updateImage();
};

class EXChannelSlider : public QWidget
{
    Q_OBJECT

public:
    EXChannelSlider(int channelIndex,
                    ColorModelSP colorModel,
                    QButtonGroup *group = nullptr,
                    QWidget *parent = nullptr);

    void setCanvas(KisCanvas2 *canvas);
    void setActive(bool active);
    void setSelected(bool selected);
    void setColor(QVector3D color, ColorModelSP colorModel);
    void setSanitizeOutOfGamut(bool sanitize, QVector3D outOfGamutColor = QVector3D());
    void setShowChannelSpinBoxes(bool show);
    void setColorConverter(EXColorConverterSP converter);
    void setColorful(bool colorful);
    void setDynamicRange(float dynamicRange);
    void setUseHdr(bool use);
    QPair<ColorModelSP, quint32> colorModelAndChannelIndex() const;
    void updateImage();

    QVector3D colorAtCurrentModel() const;
    EXChannelSliderBar *bar() const;

Q_SIGNALS:
    void sigSelected();

private:
    quint32 m_channelIndex;
    QRadioButton *m_radioButton;
    QLabel *m_label;
    QDoubleSpinBox *m_spinBox;
    EXChannelSliderBar *m_bar;
    ColorModelSP m_colorModel;
    bool m_activable;

    void updateSpinBoxRangeAndValue();
};

class EXChannelSliders : public QWidget
{
    Q_OBJECT

public:
    EXChannelSliders(ColorModelSP colorModel, QWidget *parent = nullptr);

    void setCanvas(KisCanvas2 *canvas);
    void setActive(bool active);
    const QVector<EXChannelSlider *> &sliders() const;

private:
    QVector<EXChannelSlider *> m_channelWidgets;
};

class EXChannelSlidersGroup : public QWidget
{
    Q_OBJECT
public:
    EXChannelSlidersGroup(QVector<ColorModelId> colorModels, QWidget *parent = nullptr);
    ~EXChannelSlidersGroup() override = default;

    void setCanvas(KisCanvas2 *canvas);
    void resetColorModels(QVector<ColorModelId> colorModels);

    const QVector<EXChannelSliders *> &sliders() const;

Q_SIGNALS:
    void sigChannelValueChanged(int channelIndex, float value, QVector3D fullColor, ColorModelSP colorModel);

private:
    QVBoxLayout *m_layout;
    QVector<EXChannelSliders *> m_sliders;
    KisCanvas2 *m_canvas;
};

#endif // ExtendedChannelSlider_H
