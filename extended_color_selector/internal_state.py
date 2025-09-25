from PyQt5.QtCore import (
    QObject,
    pyqtSignal,
    QTimer,
    pyqtSignal,
    QObject,
)
from PyQt5.QtGui import (
    QColor,
)
from krita import *  # type: ignore

from .models import (
    ColorModel,
    transferColorModel,
    SettingsPerColorModel,
    GlobalSettings,
)
from .config import SYNC_INTERVAL_MS


def getKritaColor() -> tuple[tuple[float, float, float], ColorModel] | None:
    kritaWindow = Krita.instance().activeWindow()  # type: ignore
    if kritaWindow == None:
        return
    kritaView = kritaWindow.activeView()  # type: ignore
    if kritaView == None:
        return
    mc = kritaView.foregroundColor()
    if mc == None:
        return

    colorModel, color = None, None
    c = mc.componentsOrdered()
    match mc.colorModel():
        case "RGBA":
            colorModel, color = ColorModel.Rgb, (c[0], c[1], c[2])
        case "LABA":
            colorModel, color = ColorModel.Lab, (c[0], c[1], c[2])
        case "XYZA":
            colorModel, color = ColorModel.Xyz, (c[0], c[1], c[2])
        case "A":
            colorModel, color = ColorModel.Rgb, (c[0], c[0], c[0])
        case "GRAYA":
            colorModel, color = ColorModel.Rgb, (c[0], c[0], c[0])
        case _:
            return None

    return (
        colorModel.normalize(color),
        colorModel,
    )


class InternalState(QObject):
    colorChanged = pyqtSignal()
    colorModelChanged = pyqtSignal()
    primaryChannelIndexChanged = pyqtSignal()
    settingsChanged = pyqtSignal()

    def __init__(self) -> None:
        super().__init__()
        self.color = 0.0, 0.0, 0.0
        self.colorModel = ColorModel.Rgb
        self.primaryIndex = 0
        self.settings = dict([(cm, SettingsPerColorModel(cm)) for cm in ColorModel])
        self.globalSettings = GlobalSettings()
        self.suppressColorSyncing = False
        self.lockedChannelBits = 0

        if self.settings[self.globalSettings.currentColorModel].enabled:
            self.updateColorModel(self.globalSettings.currentColorModel)
        else:
            for colorModel in ColorModel:
                if self.settings[colorModel].enabled:
                    self.updateColorModel(colorModel)
                    break
            else:
                self.settings[ColorModel.Rgb].enabled = True

        self.syncTimer = QTimer()
        self.syncTimer.timeout.connect(self.syncColor)
        self.syncTimer.start(SYNC_INTERVAL_MS)
        self.syncColor()

    def currentSettings(self):
        return self.settings[self.colorModel]

    def updatePrimaryIndex(self, channel: int):
        self.currentSettings().primaryIndex = channel
        self.primaryIndex = channel
        self.primaryChannelIndexChanged.emit()

    def primaryValue(self) -> float:
        return self.color[self.primaryIndex]

    def secondaryValues(self) -> tuple[float, float]:
        match self.primaryIndex:
            case 0:
                return self.color[1], self.color[2]
            case 1:
                return self.color[0], self.color[2]
            case 2:
                return self.color[0], self.color[2]
            case _:
                raise Exception("Unreachable")

    def setChannelLocked(self, channel: int, locked: bool):
        if locked:
            self.lockedChannelBits |= 1 << channel
        else:
            self.lockedChannelBits &= ~(1 << channel)

    def resolveLocked(
        self, color: tuple[float, float, float]
    ) -> tuple[float, float, float]:
        resolved = [color[0], color[1], color[2]]
        for i in range(3):
            if (self.lockedChannelBits >> i) & 1 != 0:
                resolved[i] = self.color[i]
        return resolved[0], resolved[1], resolved[2]

    def updateColor(self, color: tuple[float, float, float]):
        self.color = self.resolveLocked(color)
        self.sendColor()
        self.colorChanged.emit()

    def updateChannelValue(self, channel: int, value: float):
        match channel:
            case 0:
                color = value, self.color[1], self.color[2]
            case 1:
                color = self.color[0], value, self.color[2]
            case 2:
                color = self.color[0], self.color[1], value
            case _:
                raise Exception("Unreachable")

        self.updateColor(color)

    def updatePrimaryValue(self, value: float):
        match self.primaryIndex:
            case 0:
                color = value, self.color[1], self.color[2]
            case 1:
                color = self.color[0], value, self.color[2]
            case 2:
                color = self.color[0], self.color[1], value
            case _:
                raise Exception("Unreachable")

        self.updateColor(color)

    def updateSecondaryValues(self, variables: tuple[float, float]):
        match self.primaryIndex:
            case 0:
                color = self.color[0], variables[0], variables[1]
            case 1:
                color = variables[0], self.color[1], variables[1]
            case 2:
                color = variables[0], variables[1], self.color[2]
            case _:
                raise Exception("Unreachable")

        self.updateColor(color)

    def updateColorModel(self, colorModel: ColorModel):
        if colorModel == self.colorModel:
            return

        self.color = transferColorModel(self.color, self.colorModel, colorModel)
        self.colorModel = colorModel
        self.updatePrimaryIndex(self.currentSettings().primaryIndex)
        self.syncColor()
        self.colorModelChanged.emit()
        self.globalSettings.currentColorModel = colorModel
        self.globalSettings.write()

    def sendColor(self):
        kritaWindow = Krita.instance().activeWindow()  # type: ignore
        if kritaWindow == None:
            return
        kritaView = kritaWindow.activeView()  # type: ignore
        if kritaView == None:
            return

        kritaColor = getKritaColor()
        if kritaColor == None:
            return

        kritaColor, kritaColorModel = kritaColor
        r, g, b = transferColorModel(
            self.color,
            self.colorModel,
            ColorModel.Rgb,
            transferColorModel(kritaColor, kritaColorModel, ColorModel.Rgb),
        )
        r = min(int(r * 256), 255)
        g = min(int(g * 256), 255)
        b = min(int(b * 256), 255)
        color = ManagedColor.fromQColor(  # type: ignore
            QColor(r, g, b), kritaView.canvas()
        )
        kritaView.setForeGroundColor(color)

    def syncColor(self):
        if self.suppressColorSyncing:
            return

        kritaColor = getKritaColor()
        if kritaColor == None:
            return
        kritaColor, colorModel = kritaColor

        if STATE.globalSettings.dontSyncIfOutOfGamut:
            curColor = transferColorModel(
                self.color, self.colorModel, colorModel, clamp=False
            )
            if (
                curColor[0] > 1 + 1e-4
                or curColor[0] < -1e-4
                or curColor[1] > 1 + 1e-4
                or curColor[1] < -1e-4
                or curColor[2] > 1 + 1e-4
                or curColor[2] < -1e-4
            ):
                return

        color = transferColorModel(
            kritaColor,
            colorModel,
            self.colorModel,
            self.color,
        )

        self.color = self.resolveLocked(color)
        self.colorChanged.emit()


STATE = InternalState()
