#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "ColorState.h"
#include "ExtendedShape.h"

class SecondaryChannelsPlane : public QWidget
{
    Q_OBJECT

public:
    explicit SecondaryChannelsPlane(QWidget *parent);

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void setCanvas(KisCanvas2 *canvas);

private:
    ColorStateSP m_colorState;
    QColor m_imageColor;
    QScopedPointer<Shape> m_shape;
    QImage m_image;
    KoColorDisplayRendererInterface *m_dri;

    void updateImage();
};

#endif // COLORWHEEL_H
