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
    QVector2D,
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
import math

from .models import ColorModel


class WheelShape(Enum):
    Square = 0
    Triangle = 1
    Circle = 2

    def modifyShader(self, shader: str) -> str:
        name = None
        match self:
            case WheelShape.Square:
                name = "square"
            case WheelShape.Triangle:
                name = "triangle"
            case WheelShape.Circle:
                name = "circle"

        component = open(
            Path(__file__).parent / "shader_components" / "shapes" / f"{name}.glsl"
        ).read()[
            18:
        ]  # To strip the version directive
        return shader.replace(
            "vec2 getColorCoord(vec2 uv);",
            component,
        )

    def getColorCoord(self, uv: tuple[float, float]) -> tuple[float, float]:
        x, y = uv
        match self:
            case WheelShape.Square:
                return x, y
            case WheelShape.Triangle:
                p = QVector2D(x, y) * QVector2D(2, 2) - QVector2D(1, 1)
                RAD_120 = math.pi * 120.0 / 180.0
                V0 = QVector2D(math.cos(RAD_120 * 0.0), math.sin(RAD_120 * 0.0))
                V1 = QVector2D(math.cos(RAD_120 * 1.0), math.sin(RAD_120 * 1.0))
                V2 = QVector2D(math.cos(RAD_120 * 2.0), math.sin(RAD_120 * 2.0))
                VC = (V1 + V2) / 2.0
                VH = VC - V0
                A = (V0 - V1).length()
                H = VH.length()

                y = QVector2D.dotProduct(p - V0, VH / H) / H
                b = p - (V0 * (1 - y) + V1 * y)
                if QVector2D.dotProduct(b, V2 - V1) < 0.0:
                    return -1, -1

                x = b.length() / (y * A)

                if x < 0.0 or y < 0.0 or x > 1.0 or y > 1.0:
                    return -1, -1

                return x, y
            case WheelShape.Circle:
                x, y = x * 2 - 1, y * 2 - 1
                r = math.sqrt(x * x + y * y)
                a = math.atan2(y, x) / math.pi * 0.5 + 0.5
                return (min(r, 1), a)

    def getUv(self, coord: tuple[float, float]) -> tuple[float, float]:
        x, y = coord
        match self:
            case WheelShape.Square:
                return x, y
            case WheelShape.Triangle:
                RAD_120 = math.pi * 120.0 / 180.0
                V0 = QVector2D(math.cos(RAD_120 * 0.0), math.sin(RAD_120 * 0.0))
                V1 = QVector2D(math.cos(RAD_120 * 1.0), math.sin(RAD_120 * 1.0))
                V2 = QVector2D(math.cos(RAD_120 * 2.0), math.sin(RAD_120 * 2.0))
                VC = (V1 + V2) / 2.0
                VH = VC - V0
                A = (V0 - V1).length()
                H = VH.length()

                print(x, y)
                p = (V0 * (1 - y) + V1 * y) + (V2 - V1) * y * x
                p = p * 0.5 + QVector2D(0.5, 0.5)
                return p.x(), p.y()
            case WheelShape.Circle:
                y *= 2 * math.pi
                y += math.pi
                x, y = math.cos(y) * x, math.sin(y) * x
                return x * 0.5 + 0.5, y * 0.5 + 0.5


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
        self.shape = WheelShape.Circle
        self.compileShader()
        self.constantPos = 0
        self.outOfGamut = -1, -1, -1
        self.color = 0, 0, 0
        self.swapAxes = False
        self.reverseX = False
        self.reverseY = False

    def toggleSwapAxes(self):
        self.swapAxes = not self.swapAxes
        self.update()

    def toggleReverseX(self):
        self.reverseX = not self.reverseX
        self.update()

    def toggleReverseY(self):
        self.reverseY = not self.reverseY
        self.update()

    def updateOutOfGamutColor(self, srgb: tuple[float, float, float]):
        self.outOfGamut = srgb
        self.update()

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
        y = self.res - max(min(event.pos().y(), self.res), 0)
        if self.swapAxes:
            x, y = y, x
        if self.reverseX:
            x = self.res - x
        if self.reverseY:
            y = self.res - y

        uv = x / self.res, y / self.res
        self.variablesChanged.emit(self.shape.getColorCoord(uv))
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

        x, y = self.shape.getUv((self.color[ix], self.color[iy]))
        if self.reverseX:
            x = 1.0 - x
        if self.reverseY:
            y = 1.0 - y
        if self.swapAxes:
            x, y = y, x

        y = 1 - y
        x *= self.res
        y *= self.res

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

        axesConfig = 0
        if self.swapAxes:
            axesConfig |= 1 << 0
        if self.reverseX:
            axesConfig |= 1 << 1
        if self.reverseY:
            axesConfig |= 1 << 2

        self.program.setUniformValue("axesConfig", axesConfig)
        mn, mx = self.colorModel.limits()
        self.program.setUniformValue("lim_min", mn[0], mn[1], mn[2])
        self.program.setUniformValue("lim_max", mx[0], mx[1], mx[2])
        self.program.setUniformValue(
            "outOfGamut", self.outOfGamut[0], self.outOfGamut[1], self.outOfGamut[2]
        )

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
        self.outOfGamut = -1, -1, -1
        self.color = 0, 0, 0

    def updateOutOfGamutColor(self, srgb: tuple[float, float, float]):
        self.outOfGamut = srgb
        self.update()

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
        self.program.setUniformValue(
            "outOfGamut", self.outOfGamut[0], self.outOfGamut[1], self.outOfGamut[2]
        )

        gl = self.gl
        gl.glDrawArrays(gl.GL_TRIANGLE_STRIP, 0, 4)

    def resizeGL(self, w, h):
        if self.gl is None:
            return
        self.gl.glViewport(0, 0, w, h)
