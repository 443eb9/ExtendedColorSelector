#include <ksharedconfig.h>

#include "EXSettings.h"
#include "EXUtils.h"

EXPerColorModelSettings::EXPerColorModelSettings(QString colorModel)
    : m_configGroup(KSharedConfig::openConfig()->group(EXSettingsGroupName))
    , m_colorModel(colorModel)
{
    enabled = m_configGroup.readEntry(m_colorModel + ".enabled", true);
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
    colorfulPrimaryChannel = m_configGroup.readEntry(m_colorModel + ".colorfulPrimaryChannel", true);
    clipToSrgbGamut = m_configGroup.readEntry(m_colorModel + ".clipToSrgbGamut", false);

    int shape = m_configGroup.readEntry(colorModel + ".shape", (int)EXChannelPlaneShapeId::Square);
    this->shape = static_cast<EXChannelPlaneShapeId>(shape);
}

void EXPerColorModelSettings::writeAll()
{
    m_configGroup.writeEntry(m_colorModel + ".enabled", enabled);
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
    m_configGroup.writeEntry(m_colorModel + ".colorfulPrimaryChannel", colorfulPrimaryChannel);
    m_configGroup.writeEntry(m_colorModel + ".clipToSrgbGamut", clipToSrgbGamut);
    m_configGroup.sync();
}

EXGlobalSettings::EXGlobalSettings()
    : m_configGroup(KSharedConfig::openConfig()->group(EXSettingsGroupName))
{
    outOfGamutColorEnabled = m_configGroup.readEntry("outOfGamutColorEnabled", true);
    pWidth = m_configGroup.readEntry("pWidth", 300.0f);
    pBarHeight = m_configGroup.readEntry("pBarHeight", 20.0f);
    pEnableColorModelSwitcher = m_configGroup.readEntry("pEnableColorModelSwitcher", true);
    currentColorModel = m_configGroup.readEntry("currentColorModel", 0);

    auto displayOrder = m_configGroup.readEntry("displayOrder", "");
    this->displayOrder = ExtendedUtils::stringToVector<ColorModelId>(displayOrder, [](const QString &str) {
        bool ok;
        int id = str.toInt(&ok);
        return static_cast<ColorModelId>(id);
    });
    if (this->displayOrder.empty()) {
        this->displayOrder = ColorModelFactory::AllModels;
    }

    auto outOfGamutColor(m_configGroup.readEntry("outOfGamutColor", "0.5,0.5,0.5"));
    this->outOfGamutColor = ExtendedUtils::stringToColor(outOfGamutColor);
}

void EXGlobalSettings::writeAll()
{
    m_configGroup.writeEntry(
        "displayOrder",
        ExtendedUtils::vectorToString<ColorModelId>(this->displayOrder, [](const ColorModelId &id) {
            return QString::number(static_cast<int>(id));
        }));
    m_configGroup.writeEntry("outOfGamutColorEnabled", outOfGamutColorEnabled);
    m_configGroup.writeEntry("outOfGamutColor", ExtendedUtils::colorToString(outOfGamutColor));
    m_configGroup.writeEntry("pWidth", pWidth);
    m_configGroup.writeEntry("pBarHeight", pBarHeight);
    m_configGroup.writeEntry("pEnableColorModelSwitcher", pEnableColorModelSwitcher);
    m_configGroup.writeEntry("currentColorModel", currentColorModel);
    m_configGroup.sync();
}
