#include <KoColorSpaceRegistry.h>
#include <ksharedconfig.h>

#include "EXSettings.h"
#include "EXUtils.h"

EXPerColorModelSettings::EXPerColorModelSettings(QString colorModel)
    : m_configGroup(KSharedConfig::openConfig()->group(EXSettingsGroupName))
    , m_colorModel(colorModel)
{
    enabled = m_configGroup.readEntry(m_colorModel + ".enabled", true);
    slidersEnabled = m_configGroup.readEntry(m_colorModel + ".slidersEnabled", true);
    swapAxes = m_configGroup.readEntry(m_colorModel + ".swapAxes", false);
    reverseX = m_configGroup.readEntry(m_colorModel + ".reverseX", false);
    reverseY = m_configGroup.readEntry(m_colorModel + ".reverseY", false);
    rotation = m_configGroup.readEntry(m_colorModel + ".rotation", 0.0f);
    ringEnabled = m_configGroup.readEntry(m_colorModel + ".ringEnabled", false);
    ringThickness = m_configGroup.readEntry(m_colorModel + ".ringThickness", 0.0f);
    ringMargin = m_configGroup.readEntry(m_colorModel + ".ringMargin", 0.0f);
    ringRotation = m_configGroup.readEntry(m_colorModel + ".ringRotation", 0.0f);
    ringReversed = m_configGroup.readEntry(m_colorModel + ".ringReversed", false);
    planeRotateWithRing = m_configGroup.readEntry(m_colorModel + ".planeRotateWithRing", false);
    primaryIndex = m_configGroup.readEntry(m_colorModel + ".primaryIndex", 0);
    colorfulHueRing = m_configGroup.readEntry(m_colorModel + ".colorfulHueRing", true);
    clipToSrgbGamut = m_configGroup.readEntry(m_colorModel + ".clipToSrgbGamut", false);
    auto extraSliders = m_configGroup.readEntry(m_colorModel + ".extraSliders", "");
    this->extraSliders = ExtendedUtils::stringToVector<ColorModelId>(extraSliders, [](const QString &str) {
        return static_cast<ColorModelId>(str.toInt());
    });

    int shape = m_configGroup.readEntry(colorModel + ".shape", (int)EXChannelPlaneShapeId::Square);
    this->shape = static_cast<EXChannelPlaneShapeId>(shape);
}

void EXPerColorModelSettings::writeAll()
{
    m_configGroup.writeEntry(m_colorModel + ".enabled", enabled);
    m_configGroup.writeEntry(m_colorModel + ".slidersEnabled", slidersEnabled);
    m_configGroup.writeEntry(m_colorModel + ".shape", (int)shape);
    m_configGroup.writeEntry(m_colorModel + ".swapAxes", swapAxes);
    m_configGroup.writeEntry(m_colorModel + ".reverseX", reverseX);
    m_configGroup.writeEntry(m_colorModel + ".reverseY", reverseY);
    m_configGroup.writeEntry(m_colorModel + ".rotation", rotation);
    m_configGroup.writeEntry(m_colorModel + ".ringEnabled", ringEnabled);
    m_configGroup.writeEntry(m_colorModel + ".ringThickness", ringThickness);
    m_configGroup.writeEntry(m_colorModel + ".ringMargin", ringMargin);
    m_configGroup.writeEntry(m_colorModel + ".ringRotation", ringRotation);
    m_configGroup.writeEntry(m_colorModel + ".ringReversed", ringReversed);
    m_configGroup.writeEntry(m_colorModel + ".planeRotateWithRing", planeRotateWithRing);
    m_configGroup.writeEntry(m_colorModel + ".primaryIndex", primaryIndex);
    m_configGroup.writeEntry(m_colorModel + ".colorfulHueRing", colorfulHueRing);
    m_configGroup.writeEntry(m_colorModel + ".clipToSrgbGamut", clipToSrgbGamut);
    m_configGroup.writeEntry(
        m_colorModel + ".extraSliders",
        ExtendedUtils::vectorToString<ColorModelId>(this->extraSliders, [](const ColorModelId &id) {
            return QString::number(static_cast<int>(id));
        }));
    m_configGroup.sync();
}

EXGlobalSettings::EXGlobalSettings()
    : m_configGroup(KSharedConfig::openConfig()->group(EXSettingsGroupName))
{
    recordLastColorWhenMouseRelease = m_configGroup.readEntry("recordLastColorWhenMouseRelease", false);
    showChannelSpinBoxes = m_configGroup.readEntry("showChannelSpinBoxes", true);
    outOfGamutColorEnabled = m_configGroup.readEntry("outOfGamutColorEnabled", true);
    pWidth = m_configGroup.readEntry("pWidth", 300.0f);
    pEnableChannelPlane = m_configGroup.readEntry("pEnableChannelPlane", true);
    pEnableColorModelSwitcher = m_configGroup.readEntry("pEnableColorModelSwitcher", true);
    pEnableSliders = m_configGroup.readEntry("pEnableSliders", true);
    currentColorModel = m_configGroup.readEntry("currentColorModel", 0);

    auto displayOrder = m_configGroup.readEntry("displayOrder", "");
    this->displayOrder = ExtendedUtils::stringToVector<ColorModelId>(displayOrder, [](const QString &str) {
        return static_cast<ColorModelId>(str.toInt());
    });
    if (this->displayOrder.size() != ColorModelFactory::AllModels.size()) {
        this->displayOrder = ColorModelFactory::AllModels;
    }

    auto outOfGamutColor(m_configGroup.readEntry("outOfGamutColor", "0.5,0.5,0.5"));
    this->outOfGamutColor = ExtendedUtils::stringToColor(outOfGamutColor);
    useLayerColorSpace = m_configGroup.readEntry("useLayerColorSpace", true);
    auto customColorModel = m_configGroup.readEntry("customColorModel", QString());
    auto customColorDepth = m_configGroup.readEntry("customColorDepth", QString());
    if (customColorModel.isEmpty() || customColorDepth.isEmpty()) {
        customColorSpace = nullptr;
    } else {
        customColorSpace = KoColorSpaceRegistry::instance()->colorSpace(customColorModel, customColorDepth);
    }
    grayModelDesaturateModel = static_cast<ColorModelId>(
        m_configGroup.readEntry("grayModelDesaturateModel", static_cast<int>(ColorModelId::Oklab)));
    alwaysUseSrgbModelForHsvAndHsl = m_configGroup.readEntry("alwaysUseSrgbModelForHsvAndHsl", false);
}

void EXGlobalSettings::writeAll()
{
    m_configGroup.writeEntry("recordLastColorWhenMouseRelease", recordLastColorWhenMouseRelease);
    m_configGroup.writeEntry("showChannelSpinBoxes", showChannelSpinBoxes);
    m_configGroup.writeEntry(
        "displayOrder",
        ExtendedUtils::vectorToString<ColorModelId>(this->displayOrder, [](const ColorModelId &id) {
            return QString::number(static_cast<int>(id));
        }));
    m_configGroup.writeEntry("outOfGamutColorEnabled", outOfGamutColorEnabled);
    m_configGroup.writeEntry("outOfGamutColor", ExtendedUtils::colorToString(outOfGamutColor));
    m_configGroup.writeEntry("pWidth", pWidth);
    m_configGroup.writeEntry("pEnableChannelPlane", pEnableChannelPlane);
    m_configGroup.writeEntry("pEnableSliders", pEnableSliders);
    m_configGroup.writeEntry("pEnableColorModelSwitcher", pEnableColorModelSwitcher);
    m_configGroup.writeEntry("currentColorModel", currentColorModel);
    m_configGroup.writeEntry("useLayerColorSpace", useLayerColorSpace);
    if (customColorSpace) {
        m_configGroup.writeEntry("customColorModel", customColorSpace->colorModelId().id());
        m_configGroup.writeEntry("customColorDepth", customColorSpace->colorDepthId().id());
    }
    m_configGroup.writeEntry("grayModelDesaturateModel", static_cast<int>(grayModelDesaturateModel));
    m_configGroup.writeEntry("alwaysUseSrgbModelForHsvAndHsl", alwaysUseSrgbModelForHsvAndHsl);

    m_configGroup.sync();
}
