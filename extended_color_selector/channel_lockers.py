from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import (
    QWidget,
    QHBoxLayout,
    QRadioButton,
    QButtonGroup,
    QPushButton,
)

from .models import ColorModel
from .internal_state import STATE


class ChannelLockers(QWidget):
    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)

        self.mainLayout = QHBoxLayout(self)
        self.buttons = QPushButton(), QPushButton(), QPushButton()
        for i in range(3):
            btn = self.buttons[i]
            btn.setCheckable(True)
            btn.toggled.connect(lambda x, ch=i: self.buttonToggled(ch, x))
            self.mainLayout.addWidget(btn)

        self.updateChecked()
        self.colorModelChanged()

        STATE.colorModelChanged.connect(self.colorModelChanged)

    def buttonToggled(self, channel: int, locked: bool):
        STATE.setChannelLocked(channel, locked)
        self.updateChecked()

    def colorModelChanged(self):
        settings = STATE.currentSettings()
        if settings.showChannelLockers:
            self.show()
        else:
            self.hide()

        channels = STATE.colorModel.channelNames()
        for i in range(3):
            btn = self.buttons[i]
            btn.setText(f"Lock {channels[i]}")

    def updateChecked(self):
        for i in range(3):
            btn = self.buttons[i]
            btn.setChecked((STATE.lockedChannelBits >> i) & 1 != 0)
