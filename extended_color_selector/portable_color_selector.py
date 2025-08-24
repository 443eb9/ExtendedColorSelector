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


class PortableColorSelector(QDialog):
    def __init__(
        self,
        variablesChanged: pyqtBoundSignal,
        constantChanged: pyqtBoundSignal,
    ):
        super().__init__()
        # self.setWindowFlag(Qt.WindowType.Popup, True)
        self.mainLayout = QVBoxLayout(self)
        self.setFixedWidth(400)
        self.setMouseTracking(True)
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        self.installEventFilter(self)

        self.colorWheel = ColorWheel()
        self.lockedChannelBar = LockedChannelBar()

        self.mainLayout.addWidget(self.colorWheel)
        self.mainLayout.addWidget(self.lockedChannelBar)

    def showup(self):
        self.move(QCursor.pos())
        self.show()
        self.activateWindow()
        self.setFocus()

    # def eventFilter(self, a0: QObject | None, a1: QEvent | None) -> bool:
    #     if a1 != None:
    #         print(a1.type() == QEvent.Type.MouseButtonRelease)
    #     return True

    def mousePressEvent(self, a0: QMouseEvent | None) -> None:
        print("AAAAAAAAAAAAAAAA")

    def focusOutEvent(self, a0: QFocusEvent | None):
        super().focusOutEvent(a0)
        self.hide()


class PortableColorSelectorHandler(Extension):  # type: ignore
    # def __init__(
    #     self,
    #     variablesChanged: pyqtBoundSignal,
    #     constantChanged: pyqtBoundSignal,
    # ):
    def __init__(
        self
    ):
        super().__init__()
        # self.selector = PortableColorSelector(variablesChanged, constantChanged)
        # self.bindShortcut()

    def setup(self):
        pass

    # def bindShortcut(self):
    #     active = Krita.instance().activeWindow()  # type: ignore
    #     if active == None:
    #         return
    #     window: QWindow = active.qwindow()
    #     self.shortcut = QShortcut(QKeySequence("P"), window)
    #     self.shortcut.activated.connect(self.selector.showup)

    def createActions(self, window):
        # self.shortcut = QShortcut(QKeySequence("P"), window.qwindow())
        # self.shortcut.activated.connect(self.selector.showup)
        pass


Krita.instance().addExtension(PortableColorSelectorHandler())  # type: ignore
