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
    colorModelFromKrita,
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
    colorModel = colorModelFromKrita(mc.colorModel())
    if colorModel == None:
        return

    components = mc.componentsOrdered()
    return (
        colorModel.normalize((components[0], components[1], components[2])),
        colorModel,
    )


class InternalState(QObject):
    variablesChanged = pyqtSignal(object)
    constantChanged = pyqtSignal(float)
    colorModelChanged = pyqtSignal(ColorModel)
    lockedChannelIndexChanged = pyqtSignal(int)
    settingsChanged = pyqtSignal()

    def __init__(self) -> None:
        super().__init__()
        self.color = 0.0, 0.0, 0.0
        self.colorModel = ColorModel.Rgb
        self.lockedChannel = 0
        self.settings = dict([(cm, SettingsPerColorModel(cm)) for cm in ColorModel])
        self.globalSettings = GlobalSettings()
        self.suppressColorSyncing = False

        self.syncTimer = QTimer()
        self.syncTimer.timeout.connect(self.syncColor)
        self.syncTimer.start(SYNC_INTERVAL_MS)
        self.syncColor()

    def currentSettings(self):
        return self.settings[self.colorModel]

    def updateLockedChannel(self, channel: int):
        self.lockedChannel = channel
        self.lockedChannelIndexChanged.emit(channel)

    def updateChannelValue(self, channel: int, value: float):
        match channel:
            case 0:
                self.color = value, self.color[1], self.color[2]
            case 1:
                self.color = self.color[0], value, self.color[2]
            case 2:
                self.color = self.color[0], self.color[1], value

        self.constantChanged.emit(self.color[self.lockedChannel])

        match self.lockedChannel:
            case 0:
                self.variablesChanged.emit((self.color[1], self.color[2]))
            case 1:
                self.variablesChanged.emit((self.color[0], self.color[2]))
            case 2:
                self.variablesChanged.emit((self.color[0], self.color[1]))

        self.sendColor()

    def updateLockedChannelValue(self, value: float):
        match self.lockedChannel:
            case 0:
                self.color = value, self.color[1], self.color[2]
            case 1:
                self.color = self.color[0], value, self.color[2]
            case 2:
                self.color = self.color[0], self.color[1], value

        self.constantChanged.emit(value)
        self.sendColor()

    def updateVariableChannelsValue(self, variables: tuple[float, float]):
        match self.lockedChannel:
            case 0:
                self.color = self.color[0], variables[0], variables[1]
            case 1:
                self.color = variables[0], self.color[1], variables[1]
            case 2:
                self.color = variables[0], variables[1], self.color[2]

        self.variablesChanged.emit(variables)
        self.sendColor()

    def updateColorModel(self, colorModel: ColorModel):
        if colorModel == self.colorModel:
            return

        self.color = transferColorModel(self.color, self.colorModel, colorModel)
        self.colorModel = colorModel
        self.syncColor()
        self.colorModelChanged.emit(colorModel)

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

        self.constantChanged.emit(color[self.lockedChannel])
        match self.lockedChannel:
            case 0:
                self.variablesChanged.emit((color[1], color[2]))
            case 1:
                self.variablesChanged.emit((color[0], color[2]))
            case 2:
                self.variablesChanged.emit((color[0], color[1]))


STATE = InternalState()
