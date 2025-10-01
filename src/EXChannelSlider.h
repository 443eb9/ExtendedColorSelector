#ifndef ExtendedChannelSlider_H
#define ExtendedChannelSlider_H

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QVector>
#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

class ChannelValueBar : public QWidget
{
    Q_OBJECT

public:
    ChannelValueBar(int channelIndex, QWidget *parent);

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void setCanvas(KisCanvas2 *canvas);

private:
    int m_channelIndex;
    KoColorDisplayRendererInterface *m_dri;
    QImage m_image;

    void updateImage();
};

class ChannelValueWidget : public QWidget
{
public:
    ChannelValueWidget(int channelIndex, QButtonGroup *group, QWidget *parent = nullptr);

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
    EXChannelSliders(QWidget *parent);

    void setCanvas(KisCanvas2 *canvas);

private:
    ChannelValueWidget *m_channelWidgets[3];
};

#endif // ExtendedChannelSlider_H
