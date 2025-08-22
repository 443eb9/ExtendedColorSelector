from PyQt5.QtCore import pyqtSignal
from PyQt5.QtGui import QResizeEvent
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
)
from krita import DockWidget, DockWidgetFactory, DockWidgetFactoryBase  # type: ignore

from .color_wheel import ColorWheel, LockedChannelBar
from .models import ColorSpace

DOCKER_NAME = "Extended Color Selector"
DOCKER_ID = "pyKrita_extended_color_selector"


class ExtendedColorSelector(DockWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        self.colorSpace = ColorSpace.Rgb
        self.lockedChannel = 0
        self.color = 0, 0, 0

        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)

        self.colorWheel = ColorWheel(self)
        self.lockedChannelBar = LockedChannelBar(self)
        self.colorWheel.variablesChanged.connect(self.updateVariableChannelsValue)
        self.lockedChannelBar.constantChanged.connect(self.updateLockedChannelValue)

        self.colorSpaceSwitchers = QHBoxLayout(self)
        self.lockers = QHBoxLayout(self)

        self.updateColorSpaceSwitchers()
        self.updateLockers()

        self.mainLayout.addWidget(self.colorWheel)
        self.mainLayout.addWidget(self.lockedChannelBar)
        self.mainLayout.addLayout(self.colorSpaceSwitchers)
        self.mainLayout.addLayout(self.lockers)
        self.mainLayout.addStretch(1)

    def updateColorSpaceSwitchers(self):
        while True:
            widget = self.colorSpaceSwitchers.takeAt(0)
            if widget == None:
                break
            widget = widget.widget()
            if widget != None:
                widget.deleteLater()

        self.cswGroup = QButtonGroup()
        self.cswGroup.setExclusive(True)
        for colorSpace in ColorSpace:
            button = QRadioButton(colorSpace.displayName())
            button.setChecked(colorSpace == self.colorSpace)
            button.clicked.connect(lambda _, cs=colorSpace: self.updateColorSpace(cs))
            self.colorSpaceSwitchers.addWidget(button)
            self.cswGroup.addButton(button)

    def updateLockers(self):
        while True:
            widget = self.lockers.takeAt(0)
            if widget == None:
                break
            widget = widget.widget()
            if widget != None:
                widget.deleteLater()

        self.lockersGroup = QButtonGroup()
        self.lockersGroup.setExclusive(True)
        for i, channel in enumerate(self.colorSpace.channels()):
            button = QRadioButton(channel)
            button.clicked.connect(lambda _, i=i: self.updateLockedChannel(i))
            button.setChecked(self.lockedChannel == i)
            self.lockers.addWidget(button)
            self.lockersGroup.addButton(button, i)
        self.lockers.update()
        self.update()

    def updateColorSpace(self, colorSpace: ColorSpace):
        self.colorSpace = colorSpace
        self.colorWheel.updateColorSpace(colorSpace)
        self.lockedChannelBar.updateColorSpace(colorSpace)
        # TODO remember the locked channel
        self.lockedChannel = 0
        self.updateLockers()

    def updateLockedChannel(self, channel: int):
        self.colorWheel.updateLockedChannel(channel)
        self.lockedChannelBar.updateLockedChannel(channel)
        self.lockedChannel = channel

    def syncColor(self):
        self.colorWheel.updateColor(self.color)
        self.lockedChannelBar.updateColor(self.color)

    def updateLockedChannelValue(self, value: float):
        match self.lockedChannel:
            case 0:
                self.color = value, self.color[1], self.color[2]
            case 1:
                self.color = self.color[0], value, self.color[2]
            case 2:
                self.color = self.color[0], self.color[1], value

        self.syncColor()

    def updateVariableChannelsValue(self, variables: tuple[float, float]):
        match self.lockedChannel:
            case 0:
                self.color = self.color[0], variables[1], self.color[2]
            case 1:
                self.color = variables[0], self.color[1], variables[1]
            case 2:
                self.color = variables[0], variables[1], self.color[2]

        self.syncColor()

    def resizeEvent(self, e: QResizeEvent):
        self.colorWheel.resizeEvent(e)

    def canvasChanged(self, canvas):
        pass


instance = Krita.instance()  # type: ignore
dock_widget_factory = DockWidgetFactory(
    DOCKER_ID, DockWidgetFactoryBase.DockRight, ExtendedColorSelector
)

instance.addDockWidgetFactory(dock_widget_factory)
