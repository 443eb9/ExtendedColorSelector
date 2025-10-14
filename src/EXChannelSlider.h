#ifndef ExtendedChannelSlider_H
#define ExtendedChannelSlider_H

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "EXColorModel.h"
#include "EXColorPatchPopup.h"
#include "EXColorState.h"
#include "EXEditable.h"
#include "EXKoColorConverter.h"
#include "EXSettingsState.h"

class ChannelValueBar : public EXEditable
{
    Q_OBJECT

public:
    ChannelValueBar(int channelIndex,
                    ColorModelSP colorModel,
                    EXColorStateSP colorState,
                    EXSettingsStateSP settingsState,
                    EXColorPatchPopup *colorPatchPopup = nullptr,
                    QWidget *parent = nullptr);
    ~ChannelValueBar() override = default;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void startEdit(QMouseEvent *event, bool isShift) override;
    void edit(QMouseEvent *event) override;
    void shift(QMouseEvent *event, QVector2D delta) override;

    float currentWidgetCoord();

    void setCanvas(KisCanvas2 *canvas);
    void resetColorModel(ColorModelSP colorModel);

private:
    int m_channelIndex;
    KoColorDisplayRendererInterface *m_dri;
    QImage m_image;
    float m_editStart;
    EXColorPatchPopup *m_colorPatchPopup;
    EXColorStateSP m_colorState;
    EXSettingsStateSP m_settingsState;
    QVector3D m_colorAtCurrentModel;
    EXColorConverterSP m_converterAtCurrentModel;
    ColorModelSP m_colorModel;

    void updateImage();
};

class ChannelValueWidget : public QWidget
{
public:
    ChannelValueWidget(int channelIndex,
                       QButtonGroup *group,
                       ColorModelSP colorModel,
                       EXColorStateSP colorState,
                       EXSettingsStateSP settingsState,
                       EXColorPatchPopup *colorPatchPopup = nullptr,
                       QWidget *parent = nullptr);

    void setCanvas(KisCanvas2 *canvas);
    void resetColorModel(ColorModelSP colorModel);

private:
    quint32 m_channelIndex;
    QRadioButton *m_radioButton;
    QDoubleSpinBox *m_spinBox;
    ChannelValueBar *m_bar;
    QVector3D m_colorAtCurrentModel;
    ColorModelSP m_colorModel;
    EXColorStateSP m_colorState;

    void updateSpinBoxRangeAndValue();
};

class EXChannelSliders : public QWidget
{
    Q_OBJECT

public:
    EXChannelSliders(ColorModelSP colorModel,
                     EXColorStateSP colorState,
                     EXSettingsStateSP settingsState,
                     EXColorPatchPopup *colorPatchPopup = nullptr,
                     QWidget *parent = nullptr);

    void setCanvas(KisCanvas2 *canvas);
    void resetColorModel(ColorModelSP colorModel);

private:
    ChannelValueWidget *m_channelWidgets[3];
};

class EXChannelSlidersGroup : public QWidget
{
    Q_OBJECT
public:
    EXChannelSlidersGroup(QVector<ColorModelId> colorModels,
                          EXColorStateSP colorState,
                          EXSettingsStateSP settingsState,
                          EXColorPatchPopup *colorPatchPopup = nullptr,
                          QWidget *parent = nullptr);
    ~EXChannelSlidersGroup() override = default;

    void setCanvas(KisCanvas2 *canvas);
    void resetColorModels(QVector<ColorModelId> colorModels);

private:
    QVBoxLayout *m_layout;
    QVector<EXChannelSliders *> m_sliders;
    KisCanvas2 *m_canvas;

    EXColorStateSP m_colorState;
    EXSettingsStateSP m_settingsState;
    EXColorPatchPopup *m_colorPatchPopup;
};

#endif // ExtendedChannelSlider_H
