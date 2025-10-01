
#ifndef COLORSTATE_H
#define COLORSTATE_H

#include <QObject>
#include <QVector2D>
#include <QVector3D>

#include <KoColor.h>
#include <KoColorDisplayRendererInterface.h>
#include <KoColorSpace.h>
#include <kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "ExtendedColorModel.h"

class ColorState : public QObject, public KisShared
{
    Q_OBJECT

public:
    ColorState();

    qreal primaryChannelValue() const;
    void setPrimaryChannelValue(qreal value);
    QVector2D secondaryChannelValues() const;
    void setSecondaryChannelValues(const QVector2D &values);
    quint32 primaryChannelIndex() const;
    void setPrimaryChannelIndex(quint32 index);
    void setChannel(quint32 index, qreal value);

    QVector3D color() const;
    void setColor(const QVector3D &color);
    const KoColorSpace *colorSpace() const;
    void setColorSpace(const KoColorSpace *colorSpace);

    void setColorModel(ColorModelId model);
    ColorModelSP colorModel() const;

    void sendToKrita();
    void syncFromKrita();
    void setCanvas(KisCanvas2 *canvas);

    static ColorState *instance();

Q_SIGNALS:
    void sigColorChanged(const QVector3D &color);
    void sigPrimaryChannelIndexChanged(quint32 index);

private:
    QVector3D m_color;
    quint32 m_primaryChannelIndex;
    ColorModelSP m_colorModel;
    const KoColorSpace *m_currentColorSpace;
    KisCanvasResourceProvider *m_resourceProvider;
};

#endif // COLORSTATE_H
