from PyQt5.QtCore import pyqtSignal, QTimer, Qt
from PyQt5.QtGui import QResizeEvent, QColor
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
    QDoubleSpinBox,
    QSizePolicy,
)
import math
from krita import *  # type: ignore

from .color_wheel import ColorWheel, LockedChannelBar, WheelShape
from .models import ColorModel, colorModelFromKrita, transferColorModel
from .config import SYNC_INTERVAL_MS, DOCKER_NAME, DOCKER_ID
from .setting import SettingsDialog, GlobalSettingsDialog
from .portable_color_selector import PortableColorSelectorHandler
from .internal_state import STATE


class ExtendedColorSelector(DockWidget):  # type: ignore
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        self.settings = SettingsDialog()
        self.globalSettings = GlobalSettingsDialog()

        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)

        colorWheelLayout = QHBoxLayout()
        self.colorWheel = ColorWheel()
        colorWheelLayout.addWidget(self.colorWheel)

        self.lockedChannelBar = LockedChannelBar(False)

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
        self.updateChannelIndicators()

        settingsButtonLayout = QHBoxLayout()
        settingsButton = QPushButton()
        settingsButton.setIcon(Krita.instance().icon("configure"))  # type: ignore
        settingsButton.setFlat(True)
        settingsButton.clicked.connect(self.settings.show)
        globalSettingsButton = QPushButton()
        globalSettingsButton.setIcon(Krita.instance().icon("applications-system"))  # type: ignore
        globalSettingsButton.setFlat(True)
        globalSettingsButton.clicked.connect(self.globalSettings.show)
        settingsButtonLayout.addWidget(settingsButton)
        settingsButtonLayout.addStretch(1)
        settingsButtonLayout.addWidget(globalSettingsButton)

        self.mainLayout.addLayout(colorWheelLayout)
        self.mainLayout.addWidget(self.lockedChannelBar)
        self.mainLayout.addLayout(self.colorSpaceSwitchers)
        self.mainLayout.addLayout(self.lockers)
        self.mainLayout.addStretch(1)
        self.mainLayout.addLayout(settingsButtonLayout)

        STATE.settingsChanged.connect(self.updateFromSettings)
        STATE.colorModelChanged.connect(self.updateChannelIndicators)
        STATE.constantChanged.connect(self.updateChannelSpinBoxes)
        STATE.variablesChanged.connect(self.updateChannelSpinBoxes)
        self.updateFromSettings()

    def updateFromSettings(self):
        settings = STATE.currentSettings()
        if not settings.enabled:
            STATE.updateColorModel(ColorModel(STATE.globalSettings.displayOrder[0]))
            settings = STATE.settings[STATE.colorModel]

        self.updateChannelIndicators()
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
        for colorModel in [ColorModel(i) for i in STATE.globalSettings.displayOrder]:
            settings = STATE.settings[colorModel]
            if not settings.enabled:
                continue

            button = QRadioButton(colorModel.displayName())
            button.setChecked(colorModel == STATE.colorModel)
            button.clicked.connect(lambda _, cm=colorModel: STATE.updateColorModel(cm))
            self.colorSpaceSwitchers.addWidget(button)
            self.colorModelSwitchersGroup.addButton(button)

    def updateColorModel(self, colorModel: ColorModel):
        STATE.updateColorModel(colorModel)
        self.colorWheel.compileShader()
        self.lockedChannelBar.compileShader()

    def updateChannelIndicators(self):
        displayScales = STATE.colorModel.displayScales()
        displayMin, displayMax = STATE.colorModel.displayLimits()
        for i, channel in enumerate(STATE.colorModel.channels()):
            button = self.channelButtons[i]
            button.setText(channel)
            button.clicked.connect(lambda _, i=i: STATE.updateLockedChannel(i))
            button.setChecked(STATE.lockedChannel == i)

            valueBox = self.channelSpinBoxes[i]
            valueBox.setRange(displayMin[i], displayMax[i])
            valueBox.valueChanged.connect(
                lambda value, ch=i, scale=displayScales[i]: STATE.updateChannelValue(
                    ch, value / scale
                )
            )

        self.lockers.update()
        self.update()

    def updateChannelSpinBoxes(self):
        displayScale = STATE.colorModel.displayScales()
        for i in range(3):
            x = STATE.color[i] * displayScale[i]
            self.channelSpinBoxes[i].blockSignals(True)
            self.channelSpinBoxes[i].setValue(x)
            self.channelSpinBoxes[i].blockSignals(False)

    def resizeEvent(self, e: QResizeEvent):
        self.colorWheel.resizeEvent(e)

    def canvasChanged(self, canvas):
        STATE.syncColor()
        self.colorWheel.compileShader()


dock_widget_factory = DockWidgetFactory(  # type: ignore
    DOCKER_ID,
    DockWidgetFactoryBase.DockRight,  # type: ignore
    ExtendedColorSelector,  # type: ignore
)

Krita.instance().addDockWidgetFactory(dock_widget_factory)  # type: ignore
