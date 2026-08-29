#ifndef EXCHANNELPLANE_H
#define EXCHANNELPLANE_H

#include <QPointF>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QWidget>
#include <functional>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "EXColorPatchPopup.h"
#include "EXEditable.h"
#include "EXKoColorConverter.h"
#include "EXShape.h"
#include "KisGLImageF16.h"

class EXChannelPlane : public EXEditableImage
{
    Q_OBJECT

public:
    explicit EXChannelPlane(QWidget *parent = nullptr);
    ~EXChannelPlane() override = default;

    void updateImage();
    void setCanvas(KisCanvas2 *canvas);
    void setColorModel(ColorModelSP colorModel);
    void setPrimaryChannelIndex(int index);
    void setColor(QVector3D color, ColorModelSP colorModel);
    void setClipToSrgbGamut(bool clip);
    void setColorfulRing(bool colorful);
    void setColorConverter(EXColorConverterSP colorConverter);
    void setShape(EXChannelPlaneShapeSP shape);
    void setSanitizeOutOfGamut(bool sanitize, QVector3D outOfGamutColor = QVector3D());
    void setDynamicRange(float dynamicRange);
    void setUseHdr(bool use);

    ColorModelSP colorModel() const;

Q_SIGNALS:
    void sigPrimaryChannelValueSelected(float value, QVector3D fullColor);
    void sigSecondaryChannelsValueSelected(QVector2D values, QVector3D fullColor);

private:
    enum EditMode {
        Plane,
        Ring,
    };

    EditMode m_editMode;
    QPointF m_editStartWidgetCoordPx;
    EXChannelPlaneShapeSP m_shape;
    EXPrimaryChannelRing m_unnormalizedRing;

    float m_lastPrimaryChannelValue;

    ColorModelSP m_colorModel;
    int m_primaryChannelIndex;
    QVector2D m_secondaryChannelValues;
    QVector3D m_color;
    bool m_clipToSrgbGamut;
    bool m_colorfulRing;
    EXColorConverterSP m_converter;
    bool m_sanitizeOutOfGamut;
    QVector3D m_outOfGamutColor;
    float m_dynamicRange;
    bool m_imageDirty;

    void handleCursorEdit(const QPointF &widgetCoord);
    void sendPlaneColor(const QPointF &widgetCoord);
    void sendRingColor(const QPointF &widgetCoord);
    void offsetWidgetCoord(QPointF &widgetCoord);
    void unoffsetWidgetCoord(QPointF &widgetCoord);

    void updateSecondaryChannelValues();
    void setSecondaryChannelValues(QVector2D values);

    void updateNormalizedRing();

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

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
