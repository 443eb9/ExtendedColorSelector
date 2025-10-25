
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
#include <kis_display_color_converter.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>

#include "EXChannelPlane.h"
#include "EXChannelSlider.h"
#include "EXColorModel.h"
#include "EXKoColorConverter.h"

class EXColorState : public QObject, public KisShared
{
    Q_OBJECT

public:
    EXColorState();
    ~EXColorState() override = default;

    void setPrimaryChannelValue(float value);
    void setSecondaryChannelValues(const QVector2D &values);
    quint32 primaryChannelIndex() const;
    void setPrimaryChannelIndex(quint32 index);

    QVector3D color() const;
    QColor qColor() const;
    KoColor koColor() const;
    void setColor(const QVector3D &color);
    const KoColorSpace *colorSpace() const;
    const ColorModelSP kritaColorModel() const;
    const EXColorConverterSP koColorConverter() const;
    void setColorSpace(const KoColorSpace *colorSpace);
    void setDynamicRange(float dynamicRange);
    float dynamicRange() const { return m_dynamicRange; }

    void setColorModel(ColorModelId model);
    const ColorModelSP colorModel() const;
    void setUseLayerColorSpace(bool use);

    void sendToKrita();
    void syncFromKrita();
    void setCanvas(KisCanvas2 *canvas);

    void connectChannelPlane(EXChannelPlane *plane);
    void connectChannelSlider(EXChannelSlider *slider);
    void clearConnectedChannelSliders();

    static EXColorState *instance();

Q_SIGNALS:
    void sigColorChanged(const QVector3D &color);
    void sigPrimaryChannelIndexChanged(quint32 index);
    void sigColorModelChanged(ColorModelId id);
    void sigColorSpaceChanged(const KoColorSpace *colorSpace);
    void sigDynamicRangeChanged(float dynamicRange);

public Q_SLOTS:
    void onDisplayConfigChanged();

private:
    QVector3D m_color;
    quint32 m_primaryChannelIndex;
    ColorModelSP m_colorModel;
    const KoColorSpace *m_currentColorSpace;
    KisCanvasResourceProvider *m_resourceProvider;
    KoColorDisplayRendererInterface *m_dri;
    KisDisplayColorConverter *m_dcc;
    EXColorConverterSP m_koColorConverter;
    bool m_blockColorSync;
    bool m_useLayerColorSpace;
    float m_dynamicRange;
};

typedef KisSharedPtr<EXColorState> EXColorStateSP;

#endif // COLORSTATE_H
