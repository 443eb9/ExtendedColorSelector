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
    QMatrix2x2,
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
            "vec2 getColorCoord(vec2 p, float normalizedRingThickness);",
            component,
        )

    def getColorCoord(
        self, p: tuple[float, float], normalizedRingThickness: float
    ) -> tuple[float, float]:
        x, y = p
        match self:
            case WheelShape.Square:
                if normalizedRingThickness == 0:
                    return x * 0.5 + 0.5, y * 0.5 + 0.5

                d = 2.0 - normalizedRingThickness * 2
                a = d / math.sqrt(2.0)
                if abs(x) > a * 0.5 or abs(y) > a * 0.5:
                    return -1, -1

                return x / (a * 0.5) * 0.5 + 0.5, y / (a * 0.5) * 0.5 + 0.5
            case WheelShape.Triangle:
                p = QVector2D(x, y)
                RAD_120 = math.pi * 120.0 / 180.0
                t = 1.0 - normalizedRingThickness
                V0 = QVector2D(math.cos(RAD_120 * 0.0), math.sin(RAD_120 * 0.0)) * t
                V1 = QVector2D(math.cos(RAD_120 * 1.0), math.sin(RAD_120 * 1.0)) * t
                V2 = QVector2D(math.cos(RAD_120 * 2.0), math.sin(RAD_120 * 2.0)) * t
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
                r = math.sqrt(x * x + y * y)
                if r > 1 - normalizedRingThickness:
                    return -1, -1
                a = math.atan2(y, x) / math.pi * 0.5 + 0.5
                return (r, a)

    def getPos(
        self, coord: tuple[float, float], normalizedRingThickness: float
    ) -> tuple[float, float]:
        x, y = coord
        match self:
            case WheelShape.Square:
                if normalizedRingThickness == 0:
                    return x * 2.0 - 1.0, y * 2.0 - 1.0

                d = 2.0 - normalizedRingThickness * 2
                a = d / math.sqrt(2.0)

                return x * a - a * 0.5, y * a - a * 0.5
            case WheelShape.Triangle:
                RAD_120 = math.pi * 120.0 / 180.0
                V0 = QVector2D(math.cos(RAD_120 * 0.0), math.sin(RAD_120 * 0.0))
                V1 = QVector2D(math.cos(RAD_120 * 1.0), math.sin(RAD_120 * 1.0))
                V2 = QVector2D(math.cos(RAD_120 * 2.0), math.sin(RAD_120 * 2.0))

                p = (V0 * (1 - y) + V1 * y) + (V2 - V1) * y * x
                return p.x(), p.y()
            case WheelShape.Circle:
                y *= 2 * math.pi
                y += math.pi
                x, y = math.cos(y) * x, math.sin(y) * x
                return x, y


vertex = open(Path(__file__).parent / "fullscreen.vert").read()
wheel_fragment = open(Path(__file__).parent / "color_wheel.frag").read()
bar_fragment = open(Path(__file__).parent / "locked_channel_bar.frag").read()


class ColorWheel(QOpenGLWidget):
    variablesChanged = pyqtSignal(object)

    def __init__(self, parent: QWidget):
        super().__init__(parent)
        self.setMinimumHeight(200)
        self.rotation = 0.0

        self.res = 1
        self.colorModel = ColorModel.Rgb
        self.shape = WheelShape.Square
        self.compileShader()
        self.constantPos = 0
        self.outOfGamut = -1, -1, -1
        self.color = 0, 0, 0
        self.swapAxes = False
        self.reverseX = False
        self.reverseY = False
        self.ringThickness = 0

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

    def updateRotation(self, radians: float):
        self.rotation = radians
        self.update()

    def updateRingThickness(self, thickness: float):
        self.ringThickness = thickness
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

        x = max(min(event.pos().x(), self.res), 0) / self.res
        y = max(min(event.pos().y(), self.res), 0) / self.res

        x, y = x * 2.0 - 1.0, y * 2.0 - 1.0
        y = -y
        s = math.sin(self.rotation)
        c = math.cos(self.rotation)
        x, y = x * c - y * s, x * s + y * c

        if self.swapAxes:
            x, y = y, x
        if self.reverseX:
            x = -x
        if self.reverseY:
            y = -y

        self.variablesChanged.emit(
            self.shape.getColorCoord((x, y), self.ringThickness / (self.res / 2))
        )
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

        x, y = self.shape.getPos(
            (self.color[ix], self.color[iy]), self.ringThickness / (self.res / 2)
        )
        if self.reverseX:
            x = -x
        if self.reverseY:
            y = -y
        if self.swapAxes:
            x, y = y, x

        s = math.sin(-self.rotation)
        c = math.cos(-self.rotation)
        x, y = x * c - y * s, y * c + x * s

        x, y = x * 0.5 + 0.5, y * 0.5 + 0.5
        y = 1 - y
        x, y = x * self.res, y * self.res

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
        self.program.setUniformValue("rotation", self.rotation)
        self.program.setUniformValue("ringThickness", float(self.ringThickness))
        variables = None
        match self.constantPos:
            case 0:
                variables = self.color[1], self.color[2]
            case 1:
                variables = self.color[0], self.color[2]
            case 2:
                variables = self.color[0], self.color[1]
            case _:
                return
        self.program.setUniformValue("variables", variables[0], variables[1])

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
