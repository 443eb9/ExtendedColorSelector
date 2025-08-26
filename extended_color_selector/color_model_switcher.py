from PyQt5.QtWidgets import QWidget, QHBoxLayout, QRadioButton, QButtonGroup

from .models import ColorModel
from .internal_state import STATE


class ColorModelSwitcher(QWidget):
    def __init__(self) -> None:
        super().__init__(None)
        self.mainLayout = QHBoxLayout(self)

        self.group = QButtonGroup()
        self.group.setExclusive(True)
        self.updateFromSettings()

        STATE.settingsChanged.connect(self.updateFromSettings)

    def updateFromSettings(self):
        while True:
            widget = self.mainLayout.takeAt(0)
            if widget == None:
                break
            widget = widget.widget()
            if widget != None:
                widget.deleteLater()

        for colorModel in [ColorModel(i) for i in STATE.globalSettings.displayOrder]:
            settings = STATE.settings[colorModel]
            if not settings.enabled:
                continue

            button = QRadioButton(colorModel.displayName())
            button.setChecked(colorModel == STATE.colorModel)
            button.clicked.connect(lambda _, cm=colorModel: STATE.updateColorModel(cm))
            self.mainLayout.addWidget(button)
            self.group.addButton(button)
