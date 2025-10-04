#ifndef ExtendedChannelSlider_H
#define ExtendedChannelSlider_H

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QVector>
#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "EXColorPatchPopup.h"
#include "EXEditable.h"

class ChannelValueBar : public EXEditable
{
    Q_OBJECT

public:
    ChannelValueBar(int channelIndex, EXColorPatchPopup *colorPatchPopup = nullptr, QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void startEdit(QMouseEvent *event, bool isShift) override;
    void edit(QMouseEvent *event) override;
    void shift(QMouseEvent *event, QVector2D delta) override;

    float currentWidgetCoord();

    void setCanvas(KisCanvas2 *canvas);

private:
    int m_channelIndex;
    KoColorDisplayRendererInterface *m_dri;
    QImage m_image;
    float m_editStart;
    EXColorPatchPopup *m_colorPatchPopup;

    void updateImage();
};

class ChannelValueWidget : public QWidget
{
public:
    ChannelValueWidget(int channelIndex,
                       QButtonGroup *group,
                       EXColorPatchPopup *colorPatchPopup = nullptr,
                       QWidget *parent = nullptr);

    void setCanvas(KisCanvas2 *canvas);

private:
    int m_channelIndex;
    QRadioButton *m_radioButton;
    QDoubleSpinBox *m_spinBox;
    ChannelValueBar *m_bar;
};

class EXChannelSliders : public QWidget
{
    Q_OBJECT

public:
    EXChannelSliders(EXColorPatchPopup *colorPatchPopup = nullptr, QWidget *parent = nullptr);

    void setCanvas(KisCanvas2 *canvas);

private:
    ChannelValueWidget *m_channelWidgets[3];
};

#endif // ExtendedChannelSlider_H
