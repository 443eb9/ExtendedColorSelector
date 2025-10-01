#ifndef EXTENDEDCOLORCONVERTER_H
#define EXTENDEDCOLORCONVERTER_H

#include <array>

#include <KoColor.h>
#include <KoColorSpace.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>

class EXColorConverter : public KisShared
{
public:
    EXColorConverter(const KoColorSpace *colorSpace);
    int *displayToMemoryPositionMapper();
    KoColor displayChannelsToKoColor(const QVector<float> &channels);
    QVector<float> koColorToDisplayChannels(const KoColor &color);

private:
    int m_displayToMemoryPosition[8];
    const KoColorSpace *m_colorSpace;
};

typedef KisSharedPtr<EXColorConverter> EXColorConverterSP;

#endif
