#ifndef EXCHANNELPLANE_H
#define EXCHANNELPLANE_H

#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "EXColorPatchPopup.h"
#include "EXEditable.h"
#include "EXKoColorConverter.h"
#include "EXShape.h"

class EXChannelPlane : public EXEditable
{
    Q_OBJECT

public:
    explicit EXChannelPlane(QWidget *parent = nullptr);
    ~EXChannelPlane() override = default;

    void updateImage();
    void setCanvas(KisCanvas2 *canvas);
    void setColorModel(ColorModelSP colorModel);
    void setPrimaryChannelIndex(int index);
    void setColor(QVector3D color);
    void setClipToSrgbGamut(bool clip);
    void setColorfulRing(bool colorful);
    void setKoColorConverter(EXColorConverterSP colorConverter);
    void setShape(EXChannelPlaneShapeSP shape);

    ColorModelSP colorModel() const;

Q_SIGNALS:
    void sigPrimaryChannelValueSelected(float value);
    void sigSecondaryChannelsValueSelected(QVector2D values);
    void sigStartColorSelection();
    void sigValuesFinalized();

private:
    enum EditMode {
        Plane,
        Ring,
    };

    EditMode m_editMode;
    QPointF m_editStartWidgetCoordPx;
    QColor m_imageColor;
    EXChannelPlaneShapeSP m_shape;
    EXPrimaryChannelRing m_unnormalizedRing;
    QImage m_image;
    KoColorDisplayRendererInterface *m_dri;

    float m_lastPrimaryChannelValue;

    ColorModelSP m_colorModel;
    int m_primaryChannelIndex;
    QVector2D m_secondaryChannelValues;
    QVector3D m_color;
    bool m_clipToSrgbGamut;
    bool m_colorfulRing;
    EXColorConverterSP m_koColorConverter;

    void handleCursorEdit(const QPointF &widgetCoord);
    void sendPlaneColor(const QPointF &widgetCoord);
    void sendRingColor(const QPointF &widgetCoord);
    void offsetWidgetCoord(QPointF &widgetCoord);
    void unoffsetWidgetCoord(QPointF &widgetCoord);

    void updateNormalizedRing();

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void startEdit(QMouseEvent *event, bool isShift) override;
    void edit(QMouseEvent *event) override;
    void shift(QMouseEvent *event, QVector2D delta) override;

    bool hasHeightForWidth() const override
    {
        return true;
    }

    int heightForWidth(int width) const override
    {
        return width;
    }

    float size() const;
};

#endif // EXCHANNELPLANE_H
