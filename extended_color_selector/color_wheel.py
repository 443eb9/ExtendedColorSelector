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

from .models import ColorSpace


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
        self.colorSpace = ColorSpace.Rgb
        self.shape = WheelShape.Square
        self.compileShader()
        self.constant = 0.0
        self.constantPos = 0
        self.color = 0, 0, 0
        self.cursorPos = 0, 0

    def updateColor(self, color: tuple[float, float, float]):
        self.color = color
        self.constant = color[self.constantPos]
        self.update()

    def updateColorSpace(self, colorSpace: ColorSpace):
        self.colorSpace = colorSpace
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
        val = x / self.res, 1.0 - y / self.res
        self.variablesChanged.emit(val)
        self.cursorPos = x, y
        self.update()

    def mousePressEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def paintEvent(self, e: QPaintEvent | None) -> None:
        super().paintEvent(e)
        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))

        a, b = self.cursorPos
        painter.drawArc(
            QRectF(a - 4, b - 4, 8, 8),
            0,
            360 * 16,
        )
        painter.setBrush(QBrush(QColor(0, 0, 0, 255)))
        painter.drawArc(
            QRectF(a - 5, b - 5, 10, 10),
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
            self.shape.modifyShader(self.colorSpace.modifyShader(wheel_fragment))
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
        self.program.setUniformValue("constant", float(self.constant))
        self.program.setUniformValue("constantPos", int(self.constantPos))
        x, y, z = self.colorSpace.scales()
        self.program.setUniformValue("scales", x, y, z)

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
        self.colorSpace = ColorSpace.Rgb
        self.shape = WheelShape.Square
        self.compileShader()
        self.variables = 0, 0
        self.constantPos = 0
        self.color = 0, 0, 0
        self.cursorPos = 0

    def updateColor(self, color: tuple[float, float, float]):
        self.color = color
        self.variables = (
            color[(self.constantPos + 1) % 3],
            color[(self.constantPos + 2) % 3],
        )
        self.update()

    def updateColorSpace(self, colorSpace: ColorSpace):
        self.colorSpace = colorSpace
        self.compileShader()
        self.update()

    def updateLockedChannel(self, lockedChannel: int):
        self.constantPos = lockedChannel
        self.variables = (
            self.color[(self.constantPos + 1) % 3],
            self.color[(self.constantPos + 2) % 3],
        )
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
        self.cursorPos = x
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

        x = self.cursorPos
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

        self.program.setUniformValue("res", int(self.res))
        self.program.setUniformValue(
            "variables", float(self.variables[0]), float(self.variables[1])
        )
        self.program.setUniformValue("constantPos", int(self.constantPos))
        x, y, z = self.colorSpace.scales()
        self.program.setUniformValue("scales", x, y, z)

        gl = self.gl
        gl.glDrawArrays(gl.GL_TRIANGLE_STRIP, 0, 4)

    def resizeGL(self, w, h):
        if self.gl is None:
            return
        self.gl.glViewport(0, 0, w, h)
