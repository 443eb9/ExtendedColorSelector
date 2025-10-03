#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "EXEditable.h"
#include "EXShape.h"

class EXChannelPlane : public EXEditable
{
    Q_OBJECT

public:
    explicit EXChannelPlane(QWidget *parent);

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void edit(QMouseEvent *event) override;
    void shift(QMouseEvent *event, QVector2D delta) override;

    float size() const;
    QPointF currentRingWidgetCoord();

    void setCanvas(KisCanvas2 *canvas);

private:
    enum EditMode {
        Plane,
        Ring,
    };

    EditMode m_editMode;
    QPointF m_editStart;
    QColor m_imageColor;
    EXPrimaryChannelRing m_ring;
    EXChannelPlaneShape *m_shape;
    QImage m_image;
    KoColorDisplayRendererInterface *m_dri;

    void updateImage();

private Q_SLOTS:
    void settingsChanged();
};

#endif // COLORWHEEL_H
