#ifndef EXTENDEDCOLOR_H
#define EXTENDEDCOLOR_H

#include <QVector3D>

#include <kis_shared.h>
#include <kis_shared_ptr.h>

typedef KisSharedPtr<class ColorConverter> ColorConverterSP;

enum ColorModel {
    Rgb = 0,
};

class ColorConverter : public KisShared
{
public:
    virtual QVector3D toXyz(const QVector3D &color) = 0;
    virtual QVector3D fromXyz(const QVector3D &color) = 0;
};

class RgbConverter : public ColorConverter
{
public:
    QVector3D toXyz(const QVector3D &color) override;
    QVector3D fromXyz(const QVector3D &color) override;
};

#endif
