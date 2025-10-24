#include "EXSettingsState.h"

static EXSettingsState *s_instance;
EXSettingsState *EXSettingsState::instance()
{
    if (!s_instance) {
        s_instance = new EXSettingsState();
    }
    return s_instance;
}

EXSettingsState::EXSettingsState()
    : QObject()
{
    globalSettings = EXGlobalSettings();

    for (const auto &colorModelId : globalSettings.displayOrder) {
        settings.append(EXPerColorModelSettings(ColorModelFactory::fromId(colorModelId)->displayName()));
    }
}

void EXSettingsState::connectChannelPlane(EXChannelPlane *plane)
{
    m_connectedChannelPlanes.append(plane);
}

void EXSettingsState::updateConnectedChannelPlanes()
{
    for (EXChannelPlane *plane : m_connectedChannelPlanes) {
        auto model = plane->colorModel();
        auto &settings = this->settings[model->id()];
        plane->setClipToSrgbGamut(settings.clipToSrgbGamut);
        plane->setColorfulRing(settings.colorfulHueRing);
        plane->setPrimaryChannelIndex(settings.primaryIndex);
        auto shape = EXShapeFactory::fromId(settings.shape);
        shape->reverseX = settings.reverseX;
        shape->reverseY = settings.reverseY;
        shape->swapAxes = settings.swapAxes;
        shape->rotateWithRing = settings.planeRotateWithRing;
        shape->setRotation(settings.rotation);
        shape->ring.margin = settings.ringMargin;
        shape->ring.thickness = settings.ringThickness;
        shape->ring.rotationOffset = settings.ringRotation;
        shape->ring.reversed = settings.ringReversed;
        plane->setShape(shape);
    }
}
