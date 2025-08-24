from PyQt5.QtCore import Qt, pyqtSignal, QTimer, pyqtBoundSignal
from PyQt5.QtGui import QCloseEvent, QResizeEvent, QColor, QMouseEvent
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
    QStackedWidget,
    QStackedLayout,
    QLabel,
    QColorDialog,
    QToolButton,
    QMessageBox,
    QGridLayout,
)
import math
from krita import *  # type: ignore

from .color_wheel import WheelShape
from .models import ColorModel
from .config import DOCKER_NAME


def getOrDefault(l: list[str], default: str) -> str:
    try:
        s = l.pop()
        return s
    except:
        return default


class SettingsPerColorModel:
    def __init__(self, settings: str) -> None:
        s = [] if len(settings) == 0 else list(reversed(settings.split(",")))
        self.enabled = getOrDefault(s, "True") == "True"
        self.barEnabled = getOrDefault(s, "True") == "True"
        self.shape = WheelShape(int(getOrDefault(s, "0")))
        self.displayChannels = getOrDefault(s, "True") == "True"
        self.swapAxes = getOrDefault(s, "False") == "True"
        self.reverseX = getOrDefault(s, "False") == "True"
        self.reverseY = getOrDefault(s, "False") == "True"
        self.rotation = float(getOrDefault(s, "0"))
        self.ringEnabled = getOrDefault(s, "False") == "True"
        self.ringThickness = float(getOrDefault(s, "0"))
        self.ringMargin = float(getOrDefault(s, "0"))
        self.ringRotation = float(getOrDefault(s, "0"))
        self.ringReversed = getOrDefault(s, "False") == "True"
        self.wheelRotateWithRing = getOrDefault(s, "False") == "True"
        self.outOfGamutColorEnabled = getOrDefault(s, "False") == "True"
        self.outOfGamutColor = (
            float(getOrDefault(s, "0.5")),
            float(getOrDefault(s, "0.5")),
            float(getOrDefault(s, "0.5")),
        )
        self.lockedChannelIndex = int(getOrDefault(s, "0"))

    def write(self, colorModel: ColorModel):
        s = [
            self.enabled,
            self.barEnabled,
            int(self.shape),
            self.displayChannels,
            self.swapAxes,
            self.reverseX,
            self.reverseY,
            self.rotation,
            self.ringEnabled,
            self.ringThickness,
            self.ringMargin,
            self.ringRotation,
            self.ringReversed,
            self.wheelRotateWithRing,
            self.outOfGamutColorEnabled,
            self.outOfGamutColor[0],
            self.outOfGamutColor[1],
            self.outOfGamutColor[2],
            self.lockedChannelIndex,
        ]
        Krita.instance().writeSetting(DOCKER_NAME, colorModel.displayName(), ",".join([str(x) for x in s]))  # type: ignore


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
    def __init__(self, parent: QWidget, settingsChanged: pyqtBoundSignal) -> None:
        super().__init__(parent)
        self.settingsChanged = settingsChanged

        self.colorModelSettings: dict[ColorModel, SettingsPerColorModel] = {}
        for colorModel in ColorModel:
            raw = Krita.instance().readSetting(DOCKER_NAME, colorModel.displayName(), "")  # type: ignore
            self.colorModelSettings[colorModel] = SettingsPerColorModel(raw)

        order: str = Krita.instance().readSetting(DOCKER_NAME, "displayOrder", "0")  # type: ignore
        orderList = order.split(",")
        self.displayOrder = (
            [int(cmi) for cmi in orderList]
            if len(orderList) == len(ColorModel)
            else list(range(len(ColorModel)))
        )

        self.setFixedSize(500, 300)
        self.mainLayout = QHBoxLayout(self)
        self.setWindowTitle("Extended Color Selector - Settings")

        pageSwitchers = QListWidget(self)
        pageSwitchers.setDropIndicatorShown(True)
        pageSwitchers.setDragDropMode(QListWidget.DragDropMode.InternalMove)
        pages = QStackedLayout()
        for colorModel, settings in self.colorModelSettings.items():
            pageSwitchers.addItem(colorModel.displayName())
            button = pageSwitchers.item(pageSwitchers.count() - 1)
            if button == None:
                return  # Never happens
            button.setFlags(button.flags() | Qt.ItemFlag.ItemIsUserCheckable)
            button.setCheckState(
                Qt.CheckState.Checked if settings.enabled else Qt.CheckState.Unchecked
            )

            page = QWidget()
            pageLayout = QVBoxLayout(self)
            page.setLayout(pageLayout)

            barEnabled = QCheckBox(f"Enable {colorModel.displayName()} Bar")
            barEnabled.setChecked(settings.barEnabled)
            barEnabled.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "barEnabled", x)
            )
            channelsSpinBoxEnabled = QCheckBox(f"Display Channel Values")
            channelsSpinBoxEnabled.setChecked(settings.displayChannels)
            channelsSpinBoxEnabled.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "displayChannels", x)
            )

            shapeButtonsAndRotLayout = QHBoxLayout(self)
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
            wheelRotationBox.valueChanged.connect(
                lambda rot, cm=colorModel: self.changeSetting(cm, "rotation", rot)
            )
            shapeButtonsAndRotLayout.addWidget(QLabel("Rotation"))
            shapeButtonsAndRotLayout.addWidget(wheelRotationBox)

            axesSettingsLayout = QHBoxLayout(self)
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
            ringThicknessBox = QDoubleSpinBox(self)
            ringThicknessBox.setValue(settings.ringThickness)
            ringThicknessBox.valueChanged.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "ringThickness", x)
            )
            ringMarginBox = QDoubleSpinBox(self)
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
            ringRotation = QDoubleSpinBox(self)
            ringRotation.setValue(settings.ringRotation)
            ringRotation.setMaximum(360)
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
            ringSettingsLayouts.addWidget(wheelRotateWithRingBox)
            ringSettingsLayouts.addLayout(ringSettingsLayout1)
            ringSettingsLayouts.addLayout(ringSettingsLayout2)

            ringEnabled = QCheckBox("Enable Ring")
            ringEnabled.setChecked(settings.ringEnabled)
            ringEnabled.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(cm, "ringEnabled", x)
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
                lambda x, cm=colorModel: self.changeSetting(
                    cm, "outOfGamutColor", (x.redF(), x.greenF(), x.blueF())
                )
            )
            outOfGamutColorPicker.enableButton.clicked.connect(
                lambda x, cm=colorModel: self.changeSetting(
                    cm, "outOfGamutColorEnabled", x
                )
            )

            pageLayout.addWidget(barEnabled)
            pageLayout.addWidget(channelsSpinBoxEnabled)
            pageLayout.addLayout(shapeButtonsAndRotLayout)
            pageLayout.addLayout(axesSettingsLayout)
            pageLayout.addWidget(ringEnabled)
            pageLayout.addLayout(ringSettingsLayouts)
            pageLayout.addWidget(outOfGamutColorPicker)
            pageLayout.addStretch(1)
            pages.addWidget(page)

        pageSwitchers.itemChanged.connect(self.handleColorModelEnabledChange)
        pageSwitchers.itemClicked.connect(
            lambda w: pages.setCurrentIndex(
                list([m.displayName() for m in ColorModel]).index(w.text())
            )
        )
        self.pageSwitchers = pageSwitchers
        model = pageSwitchers.model()
        if model != None:
            model.rowsMoved.connect(self.updateOrder)

        self.mainLayout.addWidget(pageSwitchers)
        self.mainLayout.addLayout(pages)

    def changeSetting(self, colorModel: ColorModel, name: str, value: object):
        setattr(self.colorModelSettings[colorModel], name, value)
        self.settingsChanged.emit()

    def handleColorModelEnabledChange(self, widget: QListWidgetItem):
        colorModel = ColorModel(
            [cm.displayName() for cm in ColorModel].index(widget.text())
        )
        self.colorModelSettings[colorModel].enabled = (
            widget.checkState() == Qt.CheckState.Checked
        )
        self.updateOrder()

    def updateOrder(self):
        widgets = [self.pageSwitchers.item(x) for x in range(self.pageSwitchers.count())]  # type: ignore
        if any([w == None for w in widgets]):
            return

        widgets: list[QListWidgetItem] = widgets
        names = [cm.displayName() for cm in ColorModel]
        self.displayOrder.clear()
        for colorModel in [ColorModel(names.index(w.text())) for w in widgets]:
            if self.colorModelSettings[colorModel].enabled:
                self.displayOrder.append(int(colorModel))

        self.settingsChanged.emit()

    def write(self):
        Krita.instance().writeSetting(  # type: ignore
            DOCKER_NAME, "displayOrder", ",".join([str(i) for i in self.displayOrder])
        )
        for colorModel, settings in self.colorModelSettings.items():
            settings.write(colorModel)

    def closeEvent(self, a0: QCloseEvent | None) -> None:
        if len(self.displayOrder) == 0:
            QMessageBox.warning(
                self,
                "Extended Color Selector - Warn",
                "You need to enable at least one color model.",
            )
            self.colorModelSettings[ColorModel.Rgb].enabled = True
            self.displayOrder.append(0)
            self.settingsChanged.emit()
            return
        self.write()
