
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

#include "ExtendedColor.h"

typedef KisSharedPtr<class ColorState> ColorStateSP;

class ColorState : public QObject, public KisShared
{
    Q_OBJECT

public:
    qreal primaryChannelValue() const;
    QVector2D secondaryChannelValues() const;
    quint32 primaryChannelIndex() const;

    QVector3D color() const;
    const KoColorSpace *colorSpace() const;
    ColorConverterSP converter() const;

    void sendToKrita();
    void syncFromKrita();
    void setCanvas(KisCanvas2 *canvas);

    static ColorStateSP instance();

private:
    QVector3D m_color;
    quint32 m_primaryChannelIndex;
    ColorModel m_model;
    ColorConverterSP m_converter;
    const KoColorSpace *m_currentColorSpace;
};

#endif // COLORSTATE_H
