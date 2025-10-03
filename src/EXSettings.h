#ifndef EXSETTINGS_H
#define EXSETTINGS_H

#include <QVector3D>
#include <QVector>

#include <kconfiggroup.h>

#include "EXColorModel.h"
#include "EXShape.h"

const QString EXSettingsGroupName = "ExtendedColorSelector_CPP";

class EXPerColorModelSettings
{
public:
    EXPerColorModelSettings(QString colorModel);
    void writeAll();

    bool enabled;
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

private:
    KConfigGroup m_configGroup;
    QString m_colorModel;
};

class EXGlobalSettings
{
public:
    EXGlobalSettings();
    void writeAll();

    QVector<ColorModelId> displayOrder;
    bool outOfGamutColorEnabled;
    QVector3D outOfGamutColor;
    float barHeight;
    bool dontSyncIfOutOfGamut;
    float pWidth;
    float pBarHeight;
    bool pEnableColorModelSwitcher;
    int currentColorModel;

private:
    KConfigGroup m_configGroup;
};

#endif // EXSETTINGS_H
