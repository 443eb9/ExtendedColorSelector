#ifndef EXTENDEDCOLORCONVERTER_H
#define EXTENDEDCOLORCONVERTER_H

#include <array>

#include <QVector4D>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>

class EXColorConverter : public KisShared
{
public:
    EXColorConverter(const KoColorSpace *colorSpace);
    const int *displayToMemoryPositionMapper() const;
    KoColor displayChannelsToKoColor(const QVector4D &channels) const;
    QVector4D koColorToDisplayChannels(const KoColor &color) const;

private:
    const KoColorSpace *m_colorSpace;
    int m_logicalToMemoryPosition[4];
    bool m_isRGBA;
    bool m_isLinear;
    bool m_applyGamma;
    bool m_exposureSupported;
};

typedef KisSharedPtr<EXColorConverter> EXColorConverterSP;

#endif
