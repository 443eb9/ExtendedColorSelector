#ifndef PRIMARYCHANNELBAR_H
#define PRIMARYCHANNELBAR_H

#include <QImage>
#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

class PrimaryChannelBar : public QWidget
{
    Q_OBJECT

public:
    PrimaryChannelBar(QWidget *parent);

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void setCanvas(KisCanvas2 *canvas);

private:
    KoColorDisplayRendererInterface *m_dri;
    QImage m_image;

    void updateImage();
};

#endif
