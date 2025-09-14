from PyQt5.QtCore import QEvent, Qt, QPoint
from PyQt5.QtGui import QKeyEvent, QCursor, QActionEvent, QKeySequence
from PyQt5.QtWidgets import QVBoxLayout, QDialog, QAction
from krita import *  # type: ignore

from .color_wheel import ColorWheel, LockedChannelBar, INDICATOR_BLOCKS
from .internal_state import STATE
from .color_model_switcher import ColorModelSwitcher


class PortableColorSelector(QDialog):
    def __init__(self):
        super().__init__()
        self.setWindowFlag(Qt.WindowType.FramelessWindowHint, True)
        self.mainLayout = QVBoxLayout(self)

        self.colorWheel = ColorWheel(self)
        self.lockedChannelBar = LockedChannelBar(True, self)
        self.colorModelSwitcher = ColorModelSwitcher()

        self.mainLayout.addWidget(self.colorWheel)
        self.mainLayout.addWidget(self.lockedChannelBar)
        self.mainLayout.addWidget(self.colorModelSwitcher)

        self.updateFromSettings()
        STATE.settingsChanged.connect(self.updateFromSettings)

    def updateFromSettings(self):
        size = STATE.globalSettings.pWidth
        self.setFixedWidth(size + STATE.globalSettings.pBarHeight + 2)
        self.colorWheel.setFixedSize(size, size)

        if STATE.globalSettings.pEnableColorModelSwitcher:
            self.colorModelSwitcher.show()
        else:
            self.colorModelSwitcher.hide()

    def toggle(self):
        self.updateFromSettings()
        if self.isVisible():
            self.hide()
            INDICATOR_BLOCKS.shut()
            STATE.suppressColorSyncing = False
        else:
            halfSize = QPoint(int(self.width() * 0.5), int(self.height() * 0.5))
            self.move(QCursor.pos() - halfSize)
            self.show()
            self.activateWindow()
            self.setFocus()
            STATE.suppressColorSyncing = True

    def leaveEvent(self, a0: QEvent | None) -> None:
        super().leaveEvent(a0)
        self.hide()
        INDICATOR_BLOCKS.shut()
        STATE.suppressColorSyncing = False

    def keyPressEvent(self, a0: QKeyEvent | None) -> None:
        if a0 == None:
            return

        action: QAction = Krita.instance().action("toggle_portable_color_selector")  # type: ignore
        if action.shortcut() == QKeySequence(a0.key() | int(a0.modifiers())):
            self.toggle()


class PortableColorSelectorHandler(Extension):  # type: ignore
    def __init__(self):
        super().__init__()
        self.selector = PortableColorSelector()
        STATE.settingsChanged.connect(self.updateFromSettings)

    def setup(self):
        pass

    def createActions(self, window: Window):  # type: ignore
        window.createAction("toggle_portable_color_selector").triggered.connect(self.selector.toggle)  # type: ignore


Krita.instance().addExtension(PortableColorSelectorHandler())  # type: ignore
