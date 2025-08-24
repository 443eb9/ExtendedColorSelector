from PyQt5.QtCore import pyqtSignal, QTimer
from PyQt5.QtGui import QResizeEvent, QColor
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
    QDoubleSpinBox,
)
import math
from krita import *  # type: ignore

from .color_wheel import ColorWheel, LockedChannelBar, WheelShape
from .models import ColorModel, colorModelFromKrita, transferColorModel
from .config import SYNC_INTERVAL_MS, DOCKER_NAME, DOCKER_ID
from .setting import SettingsDialog


class ExtendedColorSelector(DockWidget):  # type: ignore
    variablesChanged = pyqtSignal(object)
    constantChanged = pyqtSignal(float)
    settingsChanged = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        self.colorModel = ColorModel.Rgb
        self.lockedChannel = 0
        self.color = 0, 0, 0
        self.settings = SettingsDialog(self, self.settingsChanged)

        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)

        self.colorWheel = ColorWheel(self, self.variablesChanged, self.constantChanged)
        self.lockedChannelBar = LockedChannelBar(self, self.constantChanged)
        self.colorWheel.variablesChanged.connect(self.updateVariableChannelsValue)
        self.lockedChannelBar.constantChanged.connect(self.updateLockedChannelValue)

        self.colorSpaceSwitchers = QHBoxLayout()
        self.lockers = QHBoxLayout()

        self.updateColorModelSwitchers()
        self.lockersGroup = QButtonGroup()
        self.lockersGroup.setExclusive(True)
        self.channelSpinBoxes = QDoubleSpinBox(), QDoubleSpinBox(), QDoubleSpinBox()
        self.channelButtons = QRadioButton(), QRadioButton(), QRadioButton()
        for i in range(3):
            self.lockers.addWidget(self.channelButtons[i])
            self.lockersGroup.addButton(self.channelButtons[i], i)
            self.lockers.addWidget(self.channelSpinBoxes[i])
        self.updateLockers()

        self.settings = SettingsDialog(self, self.settingsChanged)
        settingsButtonLayout = QHBoxLayout()
        settingsButton = QPushButton()
        settingsButton.setIcon(Krita.instance().icon("configure"))  # type: ignore
        settingsButton.setFlat(True)
        settingsButton.clicked.connect(self.settings.show)
        settingsButtonLayout.addWidget(settingsButton)
        settingsButtonLayout.addStretch(1)

        self.mainLayout.addWidget(self.colorWheel)
        self.mainLayout.addWidget(self.lockedChannelBar)
        self.mainLayout.addLayout(self.colorSpaceSwitchers)
        self.mainLayout.addLayout(self.lockers)
        self.mainLayout.addStretch(1)
        self.mainLayout.addLayout(settingsButtonLayout)

        self.syncTimer = QTimer()
        self.syncTimer.timeout.connect(self.syncColor)
        self.syncTimer.start(SYNC_INTERVAL_MS)

        self.settingsChanged.connect(self.updateToSettings)
        self.updateToSettings()

    def updateToSettings(self):
        settings = self.settings.colorModelSettings[self.colorModel]
        if not settings.enabled:
            self.colorModel = ColorModel(self.settings.displayOrder[0])
            settings = self.settings.colorModelSettings[self.colorModel]

        self.updateLockedChannel(settings.lockedChannelIndex)
        self.lockedChannelBar.setMinimumHeight(int(settings.barHeight))
        self.lockedChannelBar.outOfGamut = (
            settings.outOfGamutColor if settings.outOfGamutColorEnabled else None
        )

        if settings.ringEnabled:
            self.colorWheel.ringThickness = settings.ringThickness
            self.colorWheel.ringMargin = settings.ringMargin
        else:
            self.colorWheel.ringThickness = 0.0
            self.colorWheel.ringMargin = 0.0
        self.colorWheel.swapAxes = settings.swapAxes
        self.colorWheel.reverseX = settings.reverseX
        self.colorWheel.reverseY = settings.reverseY
        self.colorWheel.ringReversed = settings.ringReversed
        self.colorWheel.ringRotation = math.radians(settings.ringRotation)
        self.colorWheel.wheelRotateWithRing = settings.wheelRotateWithRing
        self.colorWheel.outOfGamut = (
            settings.outOfGamutColor if settings.outOfGamutColorEnabled else None
        )
        self.colorWheel.rotation = math.radians(settings.rotation)
        self.colorWheel.updateColorModel(
            self.colorModel, self.settings.colorModelSettings[self.colorModel].shape
        )

        if settings.barEnabled:
            self.lockedChannelBar.show()
        else:
            self.lockedChannelBar.hide()
        self.lockedChannelBar.updateColorModel(self.colorModel)
        self.updateLockers()

        self.colorWheel.update()
        self.lockedChannelBar.update()
        self.updateColorModelSwitchers()

        if settings.displayChannels:
            self.updateChannelSpinBoxes()
            for b in self.channelSpinBoxes:
                b.show()
        else:
            for b in self.channelSpinBoxes:
                b.hide()

    def updateColorModelSwitchers(self):
        while True:
            widget = self.colorSpaceSwitchers.takeAt(0)
            if widget == None:
                break
            widget = widget.widget()
            if widget != None:
                widget.deleteLater()

        self.colorModelSwitchersGroup = QButtonGroup()
        self.colorModelSwitchersGroup.setExclusive(True)
        for colorModel in [ColorModel(i) for i in self.settings.displayOrder]:
            settings = self.settings.colorModelSettings[colorModel]
            if not settings.enabled:
                continue

            button = QRadioButton(colorModel.displayName())
            button.setChecked(colorModel == self.colorModel)
            button.clicked.connect(lambda _, cs=colorModel: self.updateColorModel(cs))
            self.colorSpaceSwitchers.addWidget(button)
            self.colorModelSwitchersGroup.addButton(button)

    def updateLockers(self):
        displayScales = self.colorModel.displayScales()
        displayMin, displayMax = self.colorModel.displayLimits()
        for i, channel in enumerate(self.colorModel.channels()):
            button = self.channelButtons[i]
            button.setText(channel)
            button.clicked.connect(lambda _, i=i: self.updateLockedChannel(i))
            button.setChecked(self.lockedChannel == i)

            valueBox = self.channelSpinBoxes[i]
            valueBox.setRange(displayMin[i], displayMax[i])
            valueBox.valueChanged.connect(
                lambda value, ch=i, scale=displayScales[i]: self.updateChannelValue(
                    ch, value / scale
                )
            )

        self.lockers.update()
        self.update()

    def updateColorModel(self, colorModel: ColorModel):
        if colorModel == self.colorModel:
            return

        self.color = transferColorModel(self.color, self.colorModel, colorModel)
        self.colorModel = colorModel
        self.updateToSettings()
        self.updateChannelSpinBoxes()
        self.syncColor()

    def updateChannelValue(self, channel: int, value: float):
        match channel:
            case 0:
                self.color = value, self.color[1], self.color[2]
            case 1:
                self.color = self.color[0], value, self.color[2]
            case 2:
                self.color = self.color[0], self.color[1], value

        self.propagateColor()
        self.sendColor()

    def updateLockedChannel(self, channel: int):
        if channel == self.lockedChannel:
            return

        self.settings.colorModelSettings[self.colorModel].lockedChannelIndex = channel
        self.colorWheel.updateLockedChannel(channel)
        self.lockedChannelBar.updateLockedChannel(channel)
        self.lockedChannel = channel

    def propagateColor(self):
        self.colorWheel.updateColor(self.color)
        self.lockedChannelBar.updateColor(self.color)

    def sendColor(self):
        kritaWindow = Krita.instance().activeWindow()  # type: ignore
        if kritaWindow == None:
            return
        kritaView = kritaWindow.activeView()  # type: ignore
        if kritaView == None:
            return

        r, g, b = transferColorModel(self.color, self.colorModel, ColorModel.Rgb)
        r = min(int(r * 256), 255)
        g = min(int(g * 256), 255)
        b = min(int(b * 256), 255)
        color = ManagedColor.fromQColor(  # type: ignore
            QColor(r, g, b), kritaView.canvas()
        )
        kritaView.setForeGroundColor(color)

    def syncColor(self):
        if (
            self.colorWheel.underMouse()
            or self.lockedChannelBar.underMouse()
            or any([b.underMouse() for b in self.channelSpinBoxes])
        ):
            return

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
        color = transferColorModel(
            colorModel.normalize((components[0], components[1], components[2])),
            colorModel,
            self.colorModel,
        )

        self.color = color
        self.propagateColor()

    def updateChannelSpinBoxes(self):
        displayScale = self.colorModel.displayScales()
        for i in range(3):
            x = self.color[i] * displayScale[i]
            self.channelSpinBoxes[i].setValue(x)

    def updateLockedChannelValue(self, value: float):
        match self.lockedChannel:
            case 0:
                self.color = value, self.color[1], self.color[2]
            case 1:
                self.color = self.color[0], value, self.color[2]
            case 2:
                self.color = self.color[0], self.color[1], value

        self.propagateColor()
        self.updateChannelSpinBoxes()
        self.sendColor()

    def updateVariableChannelsValue(self, variables: tuple[float, float]):
        match self.lockedChannel:
            case 0:
                self.color = self.color[0], variables[0], variables[1]
            case 1:
                self.color = variables[0], self.color[1], variables[1]
            case 2:
                self.color = variables[0], variables[1], self.color[2]

        self.propagateColor()
        self.updateChannelSpinBoxes()
        self.sendColor()

    def resizeEvent(self, e: QResizeEvent):
        self.colorWheel.resizeEvent(e)

    def canvasChanged(self, canvas):
        self.syncColor()
        self.colorWheel.compileShader()


instance = Krita.instance()  # type: ignore
dock_widget_factory = DockWidgetFactory(  # type: ignore
    DOCKER_ID,
    DockWidgetFactoryBase.DockRight,  # type: ignore
    ExtendedColorSelector,  # type: ignore
)

instance.addDockWidgetFactory(dock_widget_factory)
