#ifndef EXSETTINGS_H
#define EXSETTINGS_H

#include <QVector3D>
#include <QVector>

#include <KoColorSpace.h>
#include <kconfiggroup.h>

#include "EXColorModel.h"
#include "EXShape.h"

const QString EXSettingsGroupName = "Extended Color Selector CPP";

class EXPerColorModelSettings
{
public:
    EXPerColorModelSettings(QString colorModel);
    void writeAll();

    bool enabled;
    bool slidersEnabled;
    EXChannelPlaneShapeId shape;
    bool swapAxes;
    bool reverseX;
    bool reverseY;
    float rotation;
    bool ringEnabled;
    float ringThickness;
    float ringMargin;
    float ringRotation;
    bool ringReversed;
    bool planeRotateWithRing;
    int primaryIndex;
    bool colorfulHueRing;
    bool clipToSrgbGamut;
    QVector<ColorModelId> extraSliders;

private:
    KConfigGroup m_configGroup;
    QString m_colorModel;
};

class EXGlobalSettings
{
public:
    EXGlobalSettings();
    void writeAll();

    bool showChannelSpinBoxes;
    QVector<ColorModelId> displayOrder;
    bool outOfGamutColorEnabled;
    QVector3D outOfGamutColor;
    float pWidth;
    bool pEnableChannelPlane;
    bool pEnableSliders;
    bool pEnableColorModelSwitcher;
    int currentColorModel;
    bool useLayerColorSpace;
    const KoColorSpace *customColorSpace;
    ColorModelId grayModelDesaturateModel;

private:
    KConfigGroup m_configGroup;
};

#endif // EXSETTINGS_H
