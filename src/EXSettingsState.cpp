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
    applySettingsToPlane(plane);
    connect(this, &EXSettingsState::sigSettingsChanged, plane, [this, plane]() {
        applySettingsToPlane(plane);
    });
}

void EXSettingsState::applySettingsToPlane(EXChannelPlane *plane)
{
    auto model = plane->colorModel();
    auto &settings = this->settings[model->id()];
    plane->setClipToSrgbGamut(settings.clipToSrgbGamut);
    plane->setColorfulRing(settings.colorfulHueRing);
    plane->setPrimaryChannelIndex(settings.primaryIndex);
    plane->setSanitizeOutOfGamut(globalSettings.outOfGamutColorEnabled, globalSettings.outOfGamutColor);
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

void EXSettingsState::connectChannelSlider(EXChannelSlider *slider)
{
    applySettingsToSlider(slider);
    connect(this, &EXSettingsState::sigSettingsChanged, slider, [this, slider]() {
        applySettingsToSlider(slider);
    });
}

void EXSettingsState::applySettingsToSlider(EXChannelSlider *slider)
{
    auto [model, channelIndex] = slider->colorModelAndChannelIndex();
    auto &settings = this->settings[model->id()];
    slider->setSanitizeOutOfGamut(globalSettings.outOfGamutColorEnabled, globalSettings.outOfGamutColor);
    slider->setShowChannelSpinBoxes(globalSettings.showChannelSpinBoxes);
    slider->setColorful(settings.colorfulHueRing);
}
