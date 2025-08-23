from PyQt5.QtCore import pyqtSignal, QTimer
from PyQt5.QtGui import QResizeEvent, QColor, QMouseEvent
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
    QSpinBox,
    QDoubleSpinBox,
)
import math
from krita import *  # type: ignore

from .color_wheel import ColorWheel, LockedChannelBar
from .models import ColorModel, colorModelFromKrita, transferColorModel
from .config import SYNC_INTERVAL_MS

DOCKER_NAME = "Extended Color Selector"
DOCKER_ID = "pyKrita_extended_color_selector"


class ExtendedColorSelector(DockWidget):  # type: ignore
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        self.colorModel = ColorModel.Rgb
        self.lockedChannel = 0
        self.color = 0, 0, 0

        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)

        self.colorWheel = ColorWheel(self)
        self.lockedChannelBar = LockedChannelBar(self)
        self.colorWheel.variablesChanged.connect(self.updateVariableChannelsValue)
        self.lockedChannelBar.constantChanged.connect(self.updateLockedChannelValue)
        self.updateOutOfGamutColor((0.5, 0.5, 0.5))

        self.colorSpaceSwitchers = QHBoxLayout(self)
        self.lockers = QHBoxLayout(self)

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

        self.axesConfigLayout = QHBoxLayout(self)
        swapAxesButton = QPushButton("Swap Axes")
        swapAxesButton.clicked.connect(self.colorWheel.toggleSwapAxes)
        reverseXAxisButton = QPushButton("Revert X Axis")
        reverseXAxisButton.clicked.connect(self.colorWheel.toggleReverseX)
        reverseYAxisButton = QPushButton("Revert Y Axis")
        reverseYAxisButton.clicked.connect(self.colorWheel.toggleReverseY)
        self.axesConfigLayout.addWidget(swapAxesButton)
        self.axesConfigLayout.addWidget(reverseXAxisButton)
        self.axesConfigLayout.addWidget(reverseYAxisButton)

        wheelRotationBox = QSpinBox()
        wheelRotationBox.setMinimum(0)
        wheelRotationBox.setMaximum(360)
        wheelRotationBox.valueChanged.connect(
            lambda rot: self.colorWheel.updateRotation(math.radians(rot))
        )

        self.mainLayout.addWidget(self.colorWheel)
        self.mainLayout.addWidget(self.lockedChannelBar)
        self.mainLayout.addLayout(self.colorSpaceSwitchers)
        self.mainLayout.addLayout(self.lockers)
        self.mainLayout.addLayout(self.axesConfigLayout)
        self.mainLayout.addWidget(wheelRotationBox)
        self.mainLayout.addStretch(1)

        self.syncTimer = QTimer()
        self.syncTimer.timeout.connect(self.syncColor)
        self.syncTimer.start(SYNC_INTERVAL_MS)

    def updateColorModelSwitchers(self):
        while True:
            widget = self.colorSpaceSwitchers.takeAt(0)
            if widget == None:
                break
            widget = widget.widget()
            if widget != None:
                widget.deleteLater()

        self.cswGroup = QButtonGroup()
        self.cswGroup.setExclusive(True)
        for colorModel in ColorModel:
            button = QRadioButton(colorModel.displayName())
            button.setChecked(colorModel == self.colorModel)
            button.clicked.connect(lambda _, cs=colorModel: self.updateColorModel(cs))
            self.colorSpaceSwitchers.addWidget(button)
            self.cswGroup.addButton(button)

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
        self.colorWheel.updateColorModel(colorModel)
        self.lockedChannelBar.updateColorModel(colorModel)
        # TODO remember the locked channel
        self.lockedChannel = 0
        self.updateLockers()
        self.updateChannelSpinBoxes()
        self.syncColor()

    def updateOutOfGamutColor(self, srgb: tuple[float, float, float]):
        self.colorWheel.updateOutOfGamutColor(srgb)
        self.lockedChannelBar.updateOutOfGamutColor(srgb)

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


instance = Krita.instance()  # type: ignore
dock_widget_factory = DockWidgetFactory(  # type: ignore
    DOCKER_ID, DockWidgetFactoryBase.DockRight, ExtendedColorSelector  # type: ignore
)

instance.addDockWidgetFactory(dock_widget_factory)
