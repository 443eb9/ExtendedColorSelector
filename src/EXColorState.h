
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

#include "EXColorModel.h"
#include "EXKoColorConverter.h"

class EXColorState : public QObject, public KisShared
{
    Q_OBJECT

public:
    EXColorState();

    qreal primaryChannelValue() const;
    void setPrimaryChannelValue(float value);
    QVector2D secondaryChannelValues() const;
    void setSecondaryChannelValues(const QVector2D &values);
    quint32 primaryChannelIndex() const;
    void setPrimaryChannelIndex(quint32 index);
    void setChannel(quint32 index, float value);

    QVector3D color() const;
    QColor qColor() const;
    KoColor koColor() const;
    void setColor(const QVector3D &color);
    const KoColorSpace *colorSpace() const;
    const ColorModelSP kritaColorModel() const;
    const EXColorConverterSP koColorConverter() const;
    void setColorSpace(const KoColorSpace *colorSpace);

    void setColorModel(ColorModelId model);
    const ColorModelSP colorModel() const;
    bool possibleOutOfSrgb() const;

    void sendToKrita();
    void syncFromKrita();
    void setCanvas(KisCanvas2 *canvas);

    static EXColorState *instance();

Q_SIGNALS:
    void sigColorChanged(const QVector3D &color);
    void sigPrimaryChannelIndexChanged(quint32 index);
    void sigColorModelChanged(ColorModelId id);

private:
    QVector3D m_color;
    quint32 m_primaryChannelIndex;
    ColorModelSP m_colorModel;
    ColorModelSP m_kritaColorModel;
    const KoColorSpace *m_currentColorSpace;
    KisCanvasResourceProvider *m_resourceProvider;
    KoColorDisplayRendererInterface *m_dri;
    EXColorConverterSP m_koColorConverter;
    bool m_blockSync;
};

#endif // COLORSTATE_H
