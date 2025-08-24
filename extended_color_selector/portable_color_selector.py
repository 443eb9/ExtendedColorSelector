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

        STATE.settingsChanged.connect(self.updateSize)

    def updateSize(self):
        size = STATE.globalSettings.portableSelectorWidth
        self.setFixedWidth(size + STATE.globalSettings.portableSelectorBarHeight + 2)
        self.colorWheel.setFixedSize(size, size)

    def toggle(self):
        self.updateSize()
        print(self.isVisible())
        if self.isVisible():
            self.hide()
        else:
            print(self.window())
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


class PortableColorSelectorHandler(Extension):  # type: ignore
    def __init__(self):
        super().__init__()
        self.selector = PortableColorSelector()

    def setup(self):
        pass

    def createActions(self, window: Window):  # type: ignore
        self.shortcut = QShortcut(QKeySequence("P"), window.qwindow())
        self.shortcut.activated.connect(self.selector.toggle)


Krita.instance().addExtension(PortableColorSelectorHandler())  # type: ignore
