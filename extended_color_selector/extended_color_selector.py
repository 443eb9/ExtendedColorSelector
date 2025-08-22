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
        self.lockedChannel = "R"

        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)

        self.colorWheel = ColorWheel(self)
        self.lockedChannelBar = LockedChannelBar(self)

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
            button.setChecked(self.lockedChannel == channel)
            self.lockers.addWidget(button)
            self.lockersGroup.addButton(button, i)
        self.lockers.update()
        self.update()

    def updateColorSpace(self, colorSpace: ColorSpace):
        self.colorSpace = colorSpace
        self.colorWheel.updateColorSpace(colorSpace)
        self.lockedChannelBar.updateColorSpace(colorSpace)
        # TODO remember the locked channel
        self.lockedChannel = self.colorSpace.channels()[0]
        self.updateLockers()

    def updateLockedChannel(self, channel: int):
        self.colorWheel.updateLockedChannel(channel)
        self.lockedChannelBar.updateLockedChannel(channel)

    def updateLockedChannelValue(self, value: float):
        self.colorWheel.updateLockedChannelValue(value)

    def updateVariableValue(self, variables: tuple[float, float]):
        self.lockedChannelBar.updateVariableChannelValues(variables)

    def resizeEvent(self, e: QResizeEvent):
        self.colorWheel.resizeEvent(e)

    def canvasChanged(self, canvas):
        pass


instance = Krita.instance()  # type: ignore
dock_widget_factory = DockWidgetFactory(
    DOCKER_ID, DockWidgetFactoryBase.DockRight, ExtendedColorSelector
)

instance.addDockWidgetFactory(dock_widget_factory)
