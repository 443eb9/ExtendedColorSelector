from PyQt5.QtCore import Qt
from PyQt5.QtGui import QCloseEvent, QColor, QKeySequence
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
    QSpinBox,
    QDoubleSpinBox,
    QListWidget,
    QCheckBox,
    QListWidgetItem,
    QDialog,
    QStackedLayout,
    QLabel,
    QColorDialog,
    QMessageBox,
    QGroupBox,
    QLineEdit,
)
from krita import *  # type: ignore

from .models import ColorModel, WheelShape
from .config import *
from .internal_state import STATE


class OptionalColorPicker(QWidget):
    def __init__(self, parent: QWidget, text: str, defaultColor: QColor):
        super().__init__(parent)

        self.mainLayout = QHBoxLayout(self)
        self.indicator = QPushButton()
        self.updateColor(defaultColor)
        self.dialog = QColorDialog()
        self.enableButton = QCheckBox(text)
        self.enableButton.clicked.connect(self.enableChanged)
        self.cachedColor = defaultColor

        self.mainLayout.addWidget(self.enableButton)
        self.mainLayout.addWidget(self.indicator)

        self.indicator.clicked.connect(self.dialog.show)
        self.indicator.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.dialog.colorSelected.connect(self.updateColor)

    def enableChanged(self, enabled: bool):
        if enabled:
            self.indicator.show()
        else:
            self.indicator.hide()
        self.enableButton.setChecked(enabled)

    def updateColor(self, color: QColor):
        self.cachedColor = color
        self.indicator.setStyleSheet(
            "QPushButton {{ background-color: {0}; border: none; }}".format(
                color.name()
            )
        )


class SettingsDialog(QDialog):
    def __init__(self) -> None:
        super().__init__()
        self.setMinimumSize(SETTINGS_DIALOG_SIZE[0], SETTINGS_DIALOG_SIZE[1])
        self.mainLayout = QHBoxLayout(self)
        self.setWindowTitle("Extended Color Selector - Settings")

        pageSwitchers = QListWidget()
        pageSwitchers.setDropIndicatorShown(True)
        pageSwitchers.setDragDropMode(QListWidget.DragDropMode.InternalMove)
        pages = QStackedLayout()

        for colorModel in [ColorModel(i) for i in STATE.globalSettings.displayOrder]:
            settings = STATE.settings[colorModel]

            pageSwitchers.addItem(colorModel.displayName())
            button = pageSwitchers.item(pageSwitchers.count() - 1)
            if button == None:
                return  # Never happens
            button.setFlags(button.flags() | Qt.ItemFlag.ItemIsUserCheckable)
            button.setCheckState(
                Qt.CheckState.Checked if settings.enabled else Qt.CheckState.Unchecked
            )

            page = QWidget()
            pageLayout = QVBoxLayout()
            page.setLayout(pageLayout)

            barEnabled = QCheckBox(f"Enable {colorModel.displayName()} Bar")
            barEnabled.setChecked(settings.barEnabled)
            barEnabled.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "barEnabled", x)
            )

            colorfulLockedChannel = QCheckBox("Colorful Locked Channel")
            colorfulLockedChannel.setChecked(settings.colorfulLockedChannel)
            colorfulLockedChannel.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(
                    cm, "colorfulLockedChannel", x
                )
            )

            channelsSpinBoxEnabled = QCheckBox(f"Display Channel Values")
            channelsSpinBoxEnabled.setChecked(settings.displayChannels)
            channelsSpinBoxEnabled.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "displayChannels", x)
            )

            clipGamutBox = QCheckBox("Clip Gamut To SRGB Range")
            clipGamutBox.setChecked(settings.clipToSrgbGamut)
            clipGamutBox.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "clipToSrgbGamut", x)
            )

            shapeButtonsAndRotLayout = QHBoxLayout()
            shapesGroup = QButtonGroup()
            for shape in WheelShape:
                button = QRadioButton(shape.displayName())
                button.setChecked(shape == settings.shape)
                button.clicked.connect(
                    lambda _, s=shape, cm=colorModel: self.changeSetting(cm, "shape", s)
                )
                shapeButtonsAndRotLayout.addWidget(button)
                shapesGroup.addButton(button)

            wheelRotationBox = QDoubleSpinBox()
            wheelRotationBox.setMaximum(360)
            wheelRotationBox.setValue(settings.rotation)
            wheelRotationBox.valueChanged.connect(
                lambda rot, cm=colorModel: self.changeSetting(cm, "rotation", rot)
            )
            shapeButtonsAndRotLayout.addWidget(QLabel("Rotation"))
            shapeButtonsAndRotLayout.addWidget(wheelRotationBox)

            axesSettingsLayout = QHBoxLayout()
            swapAxesButton = QCheckBox("Swap Axes")
            swapAxesButton.setChecked(settings.swapAxes)
            swapAxesButton.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "swapAxes", x)
            )
            reverseXAxisButton = QCheckBox("Revert X Axis")
            reverseXAxisButton.setChecked(settings.reverseX)
            reverseXAxisButton.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "reverseX", x)
            )
            reverseYAxisButton = QCheckBox("Revert Y Axis")
            reverseYAxisButton.setChecked(settings.reverseY)
            reverseYAxisButton.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "reverseY", x)
            )
            axesSettingsLayout.addWidget(swapAxesButton)
            axesSettingsLayout.addWidget(reverseXAxisButton)
            axesSettingsLayout.addWidget(reverseYAxisButton)

            ringSettingsLayouts = QVBoxLayout()
            ringSettingsLayout1 = QHBoxLayout()
            ringThicknessBox = QDoubleSpinBox()
            ringThicknessBox.setValue(settings.ringThickness)
            ringThicknessBox.valueChanged.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "ringThickness", x)
            )
            ringMarginBox = QDoubleSpinBox()
            ringMarginBox.setValue(settings.ringMargin)
            ringMarginBox.valueChanged.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "ringMargin", x)
            )
            ringSettingsLayout1.addWidget(QLabel("Ring Thickness"))
            ringSettingsLayout1.addWidget(ringThicknessBox)
            ringSettingsLayout1.addWidget(QLabel("Ring Margin"))
            ringSettingsLayout1.addWidget(ringMarginBox)
            ringSettingsLayout2 = QHBoxLayout()
            ringReversed = QCheckBox("Ring Reversed")
            ringReversed.setChecked(settings.ringReversed)
            ringReversed.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "ringReversed", x)
            )
            ringRotation = QDoubleSpinBox()
            ringRotation.setMaximum(360)
            ringRotation.setValue(settings.ringRotation)
            ringRotation.valueChanged.connect(
                lambda rot, cm=colorModel: self.changeSetting(cm, "ringRotation", rot)
            )
            ringSettingsLayout2.addWidget(ringReversed)
            ringSettingsLayout2.addWidget(QLabel("Ring Rotation"))
            ringSettingsLayout2.addWidget(ringRotation)
            wheelRotateWithRingBox = QCheckBox("Wheel Rotate With Ring")
            wheelRotateWithRingBox.setChecked(settings.wheelRotateWithRing)
            wheelRotateWithRingBox.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(
                    cm, "wheelRotateWithRing", x
                )
            )
            ringSettingsLayouts.addLayout(ringSettingsLayout1)
            ringSettingsLayouts.addLayout(ringSettingsLayout2)
            ringSettingsLayouts.addWidget(wheelRotateWithRingBox)

            ringEnabled = QCheckBox("Enable Ring")
            ringEnabled.setChecked(settings.ringEnabled)
            ringEnabled.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "ringEnabled", x)
            )

            pageLayout.addWidget(barEnabled)
            if colorModel.isColorfulable():
                pageLayout.addWidget(colorfulLockedChannel)
            else:
                channelsSpinBoxEnabled.deleteLater()
            pageLayout.addWidget(channelsSpinBoxEnabled)
            if colorModel.isNotSrgbBased():
                pageLayout.addWidget(clipGamutBox)
            else:
                clipGamutBox.deleteLater()
            pageLayout.addLayout(shapeButtonsAndRotLayout)
            pageLayout.addLayout(axesSettingsLayout)
            pageLayout.addWidget(ringEnabled)
            pageLayout.addLayout(ringSettingsLayouts)
            pageLayout.addStretch(1)
            pages.addWidget(page)

        pageSwitchers.itemChanged.connect(self.handleColorModelEnabledChange)
        pageSwitchers.itemClicked.connect(
            lambda w: pages.setCurrentIndex(
                list(
                    [
                        m.displayName()
                        for m in [
                            ColorModel(d) for d in STATE.globalSettings.displayOrder
                        ]
                    ]
                ).index(w.text())
            )
        )
        self.pageSwitchers = pageSwitchers
        model = pageSwitchers.model()
        if model != None:
            model.rowsMoved.connect(self.updateOrder)

        self.mainLayout.addWidget(pageSwitchers)
        self.mainLayout.addLayout(pages)

    def changeSetting(self, colorModel: ColorModel, name: str, value: object):
        setattr(STATE.settings[colorModel], name, value)
        STATE.settingsChanged.emit()

    def handleColorModelEnabledChange(self, widget: QListWidgetItem):
        colorModel = ColorModel(
            [cm.displayName() for cm in ColorModel].index(widget.text())
        )
        STATE.settings[colorModel].enabled = (
            widget.checkState() == Qt.CheckState.Checked
        )
        self.updateOrder()

    def updateOrder(self):
        widgets = [self.pageSwitchers.item(x) for x in range(self.pageSwitchers.count())]  # type: ignore
        if any([w == None for w in widgets]):
            return

        widgets: list[QListWidgetItem] = widgets
        names = [cm.displayName() for cm in ColorModel]
        STATE.globalSettings.displayOrder.clear()
        for colorModel in [ColorModel(names.index(w.text())) for w in widgets]:
            STATE.globalSettings.displayOrder.append(int(colorModel))

        STATE.settingsChanged.emit()

    def write(self):
        Krita.instance().writeSetting(  # type: ignore
            DOCKER_NAME,
            "displayOrder",
            ",".join([str(i) for i in STATE.globalSettings.displayOrder]),
        )
        for colorModel, settings in STATE.settings.items():
            settings.write(colorModel)

    def closeEvent(self, a0: QCloseEvent | None) -> None:
        if len(STATE.globalSettings.displayOrder) == 0:
            QMessageBox.warning(
                self,
                "Extended Color Selector - Warn",
                "You need to enable at least one color model.",
            )
            STATE.settings[ColorModel.Rgb].enabled = True
            self.displayOrder.append(0)
            STATE.settingsChanged.emit()
            return
        self.write()


class GlobalSettingsDialog(QDialog):
    def __init__(self):
        super().__init__()
        settings = STATE.globalSettings
        self.mainLayout = QVBoxLayout(self)
        self.setWindowTitle("Extended Color Selector - Global Settings")
        self.setMinimumSize(
            GLOBAL_SETTINGS_DIALOG_SIZE[0], GLOBAL_SETTINGS_DIALOG_SIZE[1]
        )

        outOfGamutColorPicker = OptionalColorPicker(
            self,
            "Out Of Gamut Color",
            QColor(
                min(int(settings.outOfGamutColor[0] * 256), 255),
                min(int(settings.outOfGamutColor[1] * 256), 255),
                min(int(settings.outOfGamutColor[2] * 256), 255),
            ),
        )
        outOfGamutColorPicker.enableChanged(settings.outOfGamutColorEnabled)
        outOfGamutColorPicker.dialog.colorSelected.connect(
            lambda x: self.changeSetting(
                "outOfGamutColor", (x.redF(), x.greenF(), x.blueF())
            )
        )
        outOfGamutColorPicker.enableButton.clicked.connect(
            lambda x: self.changeSetting("outOfGamutColorEnabled", x)
        )

        barHeightLayout = QHBoxLayout()
        barHeightBox = QSpinBox()
        barHeightBox.setValue(settings.barHeight)
        barHeightBox.valueChanged.connect(lambda x: self.changeSetting("barHeight", x))
        barHeightLayout.addWidget(QLabel("Bar Height"))
        barHeightLayout.addWidget(barHeightBox)

        dontSyncIfOutOfGamutBox = QCheckBox(
            "Don't Sync Color From Krita If Out Of Gamut"
        )
        dontSyncIfOutOfGamutBox.setChecked(settings.dontSyncIfOutOfGamut)
        dontSyncIfOutOfGamutBox.clicked.connect(
            lambda x: self.changeSetting("dontSyncIfOutOfGamut", x)
        )

        portableSelectorSettingsGroup = QGroupBox("Portable Color Selector")
        pSettingsLayouts = QVBoxLayout()
        pSettingsLayout1 = QHBoxLayout()
        pWidthBox = QSpinBox()
        pWidthBox.setMaximum(1000)
        pWidthBox.setValue(settings.pWidth)
        pWidthBox.valueChanged.connect(lambda x: self.changeSetting("pWidth", x))
        pBarHeightBox = QSpinBox()
        pBarHeightBox.setValue(settings.pBarHeight)
        pBarHeightBox.valueChanged.connect(
            lambda x: self.changeSetting("pBarHeight", x)
        )
        pSettingsLayout1.addWidget(QLabel("Width"))
        pSettingsLayout1.addWidget(pWidthBox)
        pSettingsLayout1.addWidget(QLabel("Bar Height"))
        pSettingsLayout1.addWidget(pBarHeightBox)
        pSettingsLayout2 = QHBoxLayout()
        pEnableColorModelSwitcher = QCheckBox("Enable Color Model Switcher")
        pEnableColorModelSwitcher.setChecked(settings.pEnableColorModelSwitcher)
        pEnableColorModelSwitcher.clicked.connect(
            lambda x: self.changeSetting("pEnableColorModelSwitcher", x)
        )
        pShortcut = QLineEdit(settings.pShortcut)

        def setShortcut():
            try:
                x = pShortcut.text()
                _ = QKeySequence(x)
                self.changeSetting("pShortcut", x)
            except:
                pShortcut.setText(settings.pShortcut)
                return

        pShortcut.editingFinished.connect(setShortcut)
        pSettingsLayout2.addWidget(pEnableColorModelSwitcher)
        pSettingsLayout2.addWidget(QLabel("Shortcut"))
        pSettingsLayout2.addWidget(pShortcut)
        pSettingsLayouts.addLayout(pSettingsLayout1)
        pSettingsLayouts.addLayout(pSettingsLayout2)
        portableSelectorSettingsGroup.setLayout(pSettingsLayouts)

        self.mainLayout.addWidget(outOfGamutColorPicker)
        self.mainLayout.addWidget(dontSyncIfOutOfGamutBox)
        self.mainLayout.addLayout(barHeightLayout)
        self.mainLayout.addWidget(portableSelectorSettingsGroup)
        self.mainLayout.addStretch(1)

    def changeSetting(self, name: str, value: object):
        setattr(STATE.globalSettings, name, value)
        STATE.settingsChanged.emit()

    def closeEvent(self, a0: QCloseEvent | None) -> None:
        STATE.globalSettings.write()
