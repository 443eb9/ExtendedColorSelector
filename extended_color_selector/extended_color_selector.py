from PyQt5.QtGui import QResizeEvent
from PyQt5.QtWidgets import QWidget, QVBoxLayout
from krita import DockWidget, DockWidgetFactory, DockWidgetFactoryBase  # type: ignore

from.color_wheel import ColorWheel



DOCKER_NAME = "Extended Color Selector"
DOCKER_ID = "pyKrita_extended_color_selector"


class ExtendedColorSelector(DockWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle(DOCKER_NAME)
        container = QWidget(self)
        self.setWidget(container)
        self.mainLayout = QVBoxLayout(container)
        self.colorWheel = ColorWheel(self)
        self.mainLayout.addChildWidget(self.colorWheel)
        self.show()

    def resizeEvent(self, e: QResizeEvent):
        self.colorWheel.resizeEvent(e)

    def canvasChanged(self, canvas):
        pass


instance = Krita.instance()  # type: ignore
dock_widget_factory = DockWidgetFactory(
    DOCKER_ID, DockWidgetFactoryBase.DockRight, ExtendedColorSelector
)

instance.addDockWidgetFactory(dock_widget_factory)
