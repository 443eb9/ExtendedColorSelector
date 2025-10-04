#ifndef COLORWHEEL_H
#define COLORWHEEL_H

#include <QWidget>

#include <KoColorDisplayRendererInterface.h>
#include <kis_canvas2.h>

#include "EXColorPatchPopup.h"
#include "EXColorState.h"
#include "EXEditable.h"
#include "EXSettingsState.h"
#include "EXShape.h"

class EXChannelPlane : public EXEditable
{
    Q_OBJECT

public:
    explicit EXChannelPlane(EXColorStateSP colorState,
                            EXSettingsStateSP settingsState,
                            EXColorPatchPopup *colorPatchPopup = nullptr,
                            QWidget *parent = nullptr);

    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void startEdit(QMouseEvent *event, bool isShift) override;
    void edit(QMouseEvent *event) override;
    void shift(QMouseEvent *event, QVector2D delta) override;

    float size() const;

    void setCanvas(KisCanvas2 *canvas);

private:
    enum EditMode {
        Plane,
        Ring,
    };

    EditMode m_editMode;
    QPointF m_editStart;
    QColor m_imageColor;
    EXChannelPlaneShape *m_shape;
    QImage m_image;
    KoColorDisplayRendererInterface *m_dri;
    EXColorPatchPopup *m_colorPatchPopup;
    EXColorStateSP m_colorState;
    EXSettingsStateSP m_settingsState;

    void updateImage();
    void trySyncRingRotation();
    void handleCursorEdit(const QPointF &widgetCoord);
    void sendPlaneColor(const QPointF &widgetCoord);
    void sendRingColor(const QPointF &widgetCoord);

private Q_SLOTS:
    void settingsChanged();
};

#endif // COLORWHEEL_H
