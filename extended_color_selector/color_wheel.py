from PyQt5.QtCore import pyqtSignal, pyqtBoundSignal, QRectF, QLineF
from PyQt5.QtGui import (
    QMouseEvent,
    QOpenGLVersionProfile,
    QPaintEvent,
    QResizeEvent,
    QSurfaceFormat,
    QOpenGLShader,
    QOpenGLShaderProgram,
    QPainter,
    QBrush,
    QColor,
)
from PyQt5.QtWidgets import (
    QOpenGLWidget,
    QWidget,
    QVBoxLayout,
    QTextEdit,
    QLabel,
    QMessageBox,
    QSizePolicy,
)
from pathlib import Path
from enum import Enum

from .models import ColorModel


components = {}


class WheelShape(Enum):
    Square = 0

    def modifyShader(self, shader: str) -> str:
        name = None
        match self:
            case WheelShape.Square:
                name = "square"

        if not name in components:
            components[name] = open(
                Path(__file__).parent / "shader_components" / "shapes" / f"{name}.glsl"
            ).read()[
                18:
            ]  # To strip the version directive

        return shader.replace(
            "vec2 getColorCoord(vec2 uv);",
            components[name],
        )


vertex = open(Path(__file__).parent / "fullscreen.vert").read()
wheel_fragment = open(Path(__file__).parent / "color_wheel.frag").read()
bar_fragment = open(Path(__file__).parent / "locked_channel_bar.frag").read()


class ColorWheel(QOpenGLWidget):
    variablesChanged = pyqtSignal(object)

    def __init__(self, parent: QWidget):
        super().__init__(parent)
        self.setMinimumHeight(200)

        self.res = 1
        self.colorModel = ColorModel.Rgb
        self.shape = WheelShape.Square
        self.compileShader()
        self.constantPos = 0
        self.color = 0, 0, 0

    def updateColor(self, color: tuple[float, float, float]):
        self.color = color
        self.update()

    def updateColorModel(self, colorModel: ColorModel):
        self.colorModel = colorModel
        self.compileShader()
        self.update()

    def updateShape(self, shape: WheelShape):
        self.shape = shape
        self.compileShader()
        self.update()

    def updateLockedChannel(self, lockedChannel: int):
        self.constant = self.color[lockedChannel]
        self.constantPos = lockedChannel
        self.update()

    def resizeEvent(self, e: QResizeEvent | None):
        super().resizeEvent(e)

        if e == None:
            return

        size = min(e.size().width(), e.size().height())
        self.setFixedHeight(size)
        self.res = size
        self.update()

    def handleMouse(self, event: QMouseEvent | None):
        if event == None:
            return

        x = max(min(event.pos().x(), self.res), 0)
        y = max(min(event.pos().y(), self.res), 0)
        self.variablesChanged.emit((x / self.res, 1.0 - y / self.res))
        self.update()

    def mousePressEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def paintEvent(self, e: QPaintEvent | None):
        super().paintEvent(e)
        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))

        ix, iy = 0, 0
        match self.constantPos:
            case 0:
                ix, iy = 1, 2
            case 1:
                ix, iy = 0, 2
            case 2:
                ix, iy = 0, 1
        x, y = self.color[ix] * self.width(), (1 - self.color[iy]) * self.height()

        painter.drawArc(
            QRectF(x - 4, y - 4, 8, 8),
            0,
            360 * 16,
        )
        painter.setBrush(QBrush(QColor(0, 0, 0, 255)))
        painter.drawArc(
            QRectF(x - 5, y - 5, 10, 10),
            0,
            360 * 16,
        )

    def initializeGL(self):
        context = self.context()
        profile = QOpenGLVersionProfile()
        profile.setVersion(4, 1)
        profile.setProfile(QSurfaceFormat.CoreProfile)
        if context == None:
            return
        self.gl = context.versionFunctions(profile)
        self.compileShader()

    def compileShader(self):
        vert = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Vertex)
        vert.compileSourceCode(vertex)
        frag = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Fragment)
        frag.compileSourceCode(
            self.shape.modifyShader(self.colorModel.modifyShader(wheel_fragment))
        )
        self.program = QOpenGLShaderProgram(self.context())
        self.program.addShader(vert)
        self.program.addShader(frag)
        self.program.link()

    def paintGL(self):
        if self.gl == None:
            QMessageBox.critical(
                self,
                "Unable to get OpenGL Renderer.\n",
                "This error is originated from ExtendedColorSelector. \n"
                "As we are using OpenGL for color wheel rendering, it is required to use OpenGL for rendering.\n"
                "Please goto Settings -> Configure Krita -> Display -> Canvas Acceleration -> "
                "Preferred Renderer, and select OpenGL. \n\n"
                "Krita WILL CRASH if you enter the canvas. So please change the settings in start menu.",
            )
            return

        self.program.bind()

        self.program.setUniformValue("res", int(self.res))
        self.program.setUniformValue("constant", float(self.color[self.constantPos]))
        self.program.setUniformValue("constantPos", int(self.constantPos))
        mn, mx = self.colorModel.limits()
        self.program.setUniformValue("lim_min", mn[0], mn[1], mn[2])
        self.program.setUniformValue("lim_max", mx[0], mx[1], mx[2])

        gl = self.gl
        gl.glDrawArrays(gl.GL_TRIANGLE_STRIP, 0, 4)

    def resizeGL(self, w, h):
        if self.gl is None:
            return
        self.gl.glViewport(0, 0, w, h)


class LockedChannelBar(QOpenGLWidget):
    constantChanged = pyqtSignal(float)

    def __init__(self, parent: QWidget):
        super().__init__(parent)
        self.setMinimumHeight(50)

        self.res = 1
        self.colorSpace = ColorModel.Rgb
        self.shape = WheelShape.Square
        self.compileShader()
        self.constantPos = 0
        self.color = 0, 0, 0

    def updateColor(self, color: tuple[float, float, float]):
        self.color = color
        self.update()

    def updateColorModel(self, colorModel: ColorModel):
        self.colorSpace = colorModel
        self.compileShader()
        self.update()

    def updateLockedChannel(self, lockedChannel: int):
        self.constantPos = lockedChannel
        self.update()

    def resizeEvent(self, e: QResizeEvent | None):
        super().resizeEvent(e)

        if e == None:
            return

        self.res = e.size().width()
        self.update()

    def handleMouse(self, event: QMouseEvent | None):
        if event == None:
            return

        x = max(min(event.pos().x(), self.res), 0)
        self.constantChanged.emit(x / self.res)
        self.update()

    def mousePressEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def paintEvent(self, e: QPaintEvent | None):
        def drawHollowRect(painter: QPainter, x: int, y: int, w: int, h: int):
            painter.drawLine(QLineF(x, y, x + w, y))
            painter.drawLine(QLineF(x + w, y, x + w, y + h))
            painter.drawLine(QLineF(x + w, y + h, x, y + h))
            painter.drawLine(QLineF(x, y + h, x, y))

        super().paintEvent(e)
        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))

        x = int(self.color[self.constantPos] * self.width())
        drawHollowRect(painter, x - 4, 0, 8, self.height())
        painter.setBrush(QBrush(QColor(0, 0, 0, 255)))
        drawHollowRect(painter, x - 5, 0, 10, self.height())

    def initializeGL(self):
        context = self.context()
        profile = QOpenGLVersionProfile()
        profile.setVersion(4, 1)
        profile.setProfile(QSurfaceFormat.CoreProfile)
        if context == None:
            return
        self.gl = context.versionFunctions(profile)
        self.compileShader()

    def compileShader(self):
        vert = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Vertex)
        vert.compileSourceCode(vertex)
        frag = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Fragment)
        frag.compileSourceCode(
            self.shape.modifyShader(self.colorSpace.modifyShader(bar_fragment))
        )
        self.program = QOpenGLShaderProgram(self.context())
        self.program.addShader(vert)
        self.program.addShader(frag)
        self.program.link()

    def paintGL(self):
        if self.gl == None:
            QMessageBox.critical(
                self,
                "Unable to get OpenGL Renderer.\n",
                "This error is originated from ExtendedColorSelector. \n"
                "As we are using OpenGL for color wheel rendering, it is required to use OpenGL for rendering.\n"
                "Please goto Settings -> Configure Krita -> Display -> Canvas Acceleration -> "
                "Preferred Renderer, and select OpenGL. \n\n"
                "Krita WILL CRASH if you enter the canvas. So please change the settings in start menu.",
            )
            return

        self.program.bind()

        ix, iy = 0, 0
        match self.constantPos:
            case 0:
                ix, iy = 1, 2
            case 1:
                ix, iy = 0, 2
            case 2:
                ix, iy = 0, 1

        self.program.setUniformValue("res", int(self.res))
        self.program.setUniformValue(
            "variables",
            float(self.color[ix]),
            float(self.color[iy]),
        )
        self.program.setUniformValue("constantPos", int(self.constantPos))
        mn, mx = self.colorSpace.limits()
        self.program.setUniformValue("lim_min", mn[0], mn[1], mn[2])
        self.program.setUniformValue("lim_max", mx[0], mx[1], mx[2])

        gl = self.gl
        gl.glDrawArrays(gl.GL_TRIANGLE_STRIP, 0, 4)

    def resizeGL(self, w, h):
        if self.gl is None:
            return
        self.gl.glViewport(0, 0, w, h)
