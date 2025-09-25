from PyQt5.QtGui import QResizeEvent, QMouseEvent
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
    QDoubleSpinBox,
)
from krita import *  # type: ignore

from .color_wheel import SecondaryChannelsPlane, PrimaryChannelBar, INDICATOR_BLOCKS
from .models import ColorModel
from .config import DOCKER_NAME, DOCKER_ID
from .setting import SettingsDialog, GlobalSettingsDialog
from .internal_state import STATE
from .color_model_switcher import ColorModelSwitcher


class ExtendedColorSelector(DockWidget):  # type: ignore
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        self.settings = SettingsDialog()
        self.globalSettings = GlobalSettingsDialog()

        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)

        self.secondaryChannelsPlane = SecondaryChannelsPlane()
        self.primaryChannelBar = PrimaryChannelBar(False)
        self.colorSpaceSwitcher = ColorModelSwitcher()
        self.lockers = QHBoxLayout()

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

        self.mainLayout.addWidget(self.secondaryChannelsPlane)
        self.mainLayout.addWidget(self.primaryChannelBar)
        self.mainLayout.addWidget(self.colorSpaceSwitcher)
        self.mainLayout.addLayout(self.lockers)
        self.mainLayout.addLayout(settingsButtonLayout)
        self.mainLayout.addStretch(1)

        STATE.settingsChanged.connect(self.updateFromSettings)
        STATE.colorModelChanged.connect(self.updateColorModel)
        STATE.colorChanged.connect(self.updateChannelSpinBoxes)
        self.updateFromSettings()

    def updateFromSettings(self):
        settings = STATE.currentSettings()
        if not settings.enabled:
            STATE.updateColorModel(ColorModel(STATE.globalSettings.displayOrder[0]))
            settings = STATE.settings[STATE.colorModel]

        self.updateChannelIndicators()

        if settings.displayChannels:
            self.updateChannelSpinBoxes()
            for b in self.channelSpinBoxes:
                b.show()
        else:
            for b in self.channelSpinBoxes:
                b.hide()

    def updateColorModel(self):
        self.updateChannelIndicators()
        self.updateChannelSpinBoxes()
        self.secondaryChannelsPlane.compileShader()
        self.primaryChannelBar.compileShader()

    def updateChannelIndicators(self):
        displayMin, displayMax = STATE.colorModel.displayLimits()
        self.updateChannelSpinBoxes()

        def update(value: float, ch: int):
            STATE.suppressColorSyncing = True
            STATE.updateChannelValue(
                ch, STATE.colorModel.fromDisplayValues((value, value, value))[ch]
            )

        def finished():
            STATE.suppressColorSyncing = False

        for i, channel in enumerate(STATE.colorModel.channels()):
            button = self.channelButtons[i]
            button.setText(channel)
            button.clicked.connect(lambda _, ch=i: STATE.updatePrimaryIndex(ch))
            button.setChecked(STATE.primaryIndex == i)

            valueBox = self.channelSpinBoxes[i]
            valueBox.setRange(displayMin[i], displayMax[i])
            valueBox.valueChanged.connect(lambda value, ch=i: update(value, ch))
            valueBox.editingFinished.connect(finished)

        self.lockers.update()
        self.update()

    def updateChannelSpinBoxes(self):
        display = STATE.colorModel.toDisplayValues(STATE.color)
        for i in range(3):
            self.channelSpinBoxes[i].blockSignals(True)
            self.channelSpinBoxes[i].setValue(display[i])
            self.channelSpinBoxes[i].blockSignals(False)

    def enterEvent(self, event: QMouseEvent):
        STATE.suppressColorSyncing = True

    def leaveEvent(self, event: QMouseEvent):
        STATE.suppressColorSyncing = False
        INDICATOR_BLOCKS.shut()

    def resizeEvent(self, e: QResizeEvent):
        self.secondaryChannelsPlane.resizeEvent(e)

    def canvasChanged(self, canvas):
        STATE.syncColor()
        self.secondaryChannelsPlane.updateShaders()
        self.primaryChannelBar.updateShaders()


dock_widget_factory = DockWidgetFactory(  # type: ignore
    DOCKER_ID,
    DockWidgetFactoryBase.DockRight,  # type: ignore
    ExtendedColorSelector,  # type: ignore
)

Krita.instance().addDockWidgetFactory(dock_widget_factory)  # type: ignore
