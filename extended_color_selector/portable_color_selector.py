from PyQt5.QtCore import (
    QEvent,
    QObject,
    Qt,
    pyqtSignal,
    QTimer,
    pyqtBoundSignal,
    pyqtSignal,
)
from PyQt5.QtGui import (
    QKeyEvent,
    QMouseEvent,
    QResizeEvent,
    QColor,
    QFocusEvent,
    QCursor,
    QKeySequence,
    QWindow,
)
from PyQt5.QtWidgets import (
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QPushButton,
    QRadioButton,
    QButtonGroup,
    QDoubleSpinBox,
    QDialog,
    QAction,
    QShortcut,
)
import math
from krita import *  # type: ignore

from .color_wheel import ColorWheel, LockedChannelBar, WheelShape
from .models import ColorModel, colorModelFromKrita, transferColorModel
from .config import SYNC_INTERVAL_MS, DOCKER_NAME, DOCKER_ID
from .setting import SettingsDialog, GlobalSettingsDialog
from .internal_state import STATE


class PortableColorSelector(QDialog):
    def __init__(self):
        super().__init__()
        self.setWindowFlag(Qt.WindowType.FramelessWindowHint, True)
        self.mainLayout = QVBoxLayout(self)

        self.colorWheel = ColorWheel()
        self.lockedChannelBar = LockedChannelBar(True)

        self.mainLayout.addWidget(self.colorWheel)
        self.mainLayout.addWidget(self.lockedChannelBar)

        STATE.settingsChanged.connect(self.updateFromSettings)

    def updateFromSettings(self):
        size = STATE.globalSettings.portableSelectorWidth
        self.setFixedWidth(size + STATE.globalSettings.portableSelectorBarHeight + 2)
        self.colorWheel.setFixedSize(size, size)

    def toggle(self):
        self.updateFromSettings()
        if self.isVisible():
            self.hide()
        else:
            self.move(QCursor.pos())
            self.show()
            self.activateWindow()
            self.setFocus()

    def focusOutEvent(self, a0: QFocusEvent | None):
        super().focusOutEvent(a0)
        self.hide()

    def leaveEvent(self, a0: QEvent | None) -> None:
        super().leaveEvent(a0)
        self.hide()

    def keyPressEvent(self, a0: QKeyEvent | None) -> None:
        if a0 == None:
            return
        keys = ""
        m = int(a0.modifiers())
        if (m & Qt.KeyboardModifier.ShiftModifier) != 0:
            keys += "shift+"
        if (m & Qt.KeyboardModifier.ControlModifier) != 0:
            keys += "ctrl+"
        if (m & Qt.KeyboardModifier.AltModifier) != 0:
            keys += "alt+"
        keys += QKeySequence(a0.key()).toString()
        seq = QKeySequence(keys)
        print(keys)
        if seq == QKeySequence(STATE.globalSettings.portableSelectorShortcut):
            self.hide()


class PortableColorSelectorHandler(Extension):  # type: ignore
    def __init__(self):
        super().__init__()
        self.selector = PortableColorSelector()
        STATE.settingsChanged.connect(self.updateFromSettings)

    def updateFromSettings(self):
        self.shortcut.setKey(STATE.globalSettings.portableSelectorShortcut)

    def setup(self):
        pass

    def createActions(self, window: Window):  # type: ignore
        self.shortcut = QShortcut(
            QKeySequence(STATE.globalSettings.portableSelectorShortcut),
            window.qwindow(),
        )
        self.shortcut.activated.connect(self.selector.toggle)


Krita.instance().addExtension(PortableColorSelectorHandler())  # type: ignore
