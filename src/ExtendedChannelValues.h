#ifndef EXTENDEDCHANNELVALUES_H
#define EXTENDEDCHANNELVALUES_H

#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

class ChannelValueBar : public QWidget
{
    Q_OBJECT

public:
    ChannelValueBar(int channelIndex, QWidget *parent);

    void paintEvent(QPaintEvent *event) override;
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
    ChannelValueWidget(int channelIndex, QWidget *parent = nullptr);

    int m_channelIndex;
    QRadioButton *m_radioButton;
    QDoubleSpinBox *m_spinBox;
    ChannelValueBar *m_bar;

    void setCanvas(KisCanvas2 *canvas);
};

class ExtendedChannelValues : public QWidget
{
    Q_OBJECT

public:
    ExtendedChannelValues(QWidget *parent);

    ChannelValueWidget *m_channelWidgets[3];

    void setCanvas(KisCanvas2 *canvas);
};

#endif // EXTENDEDCHANNELVALUES_H
