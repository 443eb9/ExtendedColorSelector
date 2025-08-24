from PyQt5.QtCore import QEvent, QSize, pyqtBoundSignal, QRectF, QPoint, Qt
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
    QPalette,
)
from PyQt5.QtWidgets import QOpenGLWidget, QWidget, QMessageBox, QSizePolicy
from pathlib import Path
from enum import IntEnum
import math

from .models import ColorModel, WheelShape, SettingsPerColorModel
from .setting import SettingsPerColorModel, GlobalSettings
from .internal_state import STATE
from .config import *


vertex = open(Path(__file__).parent / "fullscreen.vert").read()
wheelFragment = open(Path(__file__).parent / "color_wheel.frag").read()
barFragment = open(Path(__file__).parent / "locked_channel_bar.frag").read()


def computeMoveFactor(e: QMouseEvent) -> float:
    m = e.modifiers()
    if m == Qt.KeyboardModifier.ShiftModifier:
        return 0.1
    if m == Qt.KeyboardModifier.AltModifier:
        return 0.01
    return 1.0


class ColorWheel(QOpenGLWidget):
    class ColorWheelEditing(IntEnum):
        Wheel = 1
        Ring = 2

    def __init__(self):
        super().__init__()
        self.editing = ColorWheel.ColorWheelEditing.Wheel
        self.gl = None
        self.program = None
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding)

        self.res = 1
        self.editStart: QVector2D | None = None
        self.setMinimumSize(MIN_WHEEL_SIZE, MIN_WHEEL_SIZE)
        self.setMaximumSize(MAX_WHEEL_SIZE, MAX_WHEEL_SIZE)

        STATE.settingsChanged.connect(self.compileShader)
        STATE.colorModelChanged.connect(self.compileShader)
        STATE.constantChanged.connect(self.update)
        STATE.lockedChannelIndexChanged.connect(self.update)

    def resizeEvent(self, e: QResizeEvent | None):
        if e == None:
            return

        size = min(e.size().width(), e.size().height())
        # self.resize(size, size)
        self.res = size
        super().resizeEvent(e)
        self.update()

    def hasHeightForWidth(self) -> bool:
        return True

    def heightForWidth(self, a0: int) -> int:
        return a0

    def sizeHint(self) -> QSize:
        size = super().sizeHint()
        x = max(size.width(), size.height())
        return QSize(x, x)

    def handleWheelEdit(self, cursor: QVector2D):
        if self.editStart == None:
            return

        settings = STATE.currentSettings()
        x = max(min(cursor.x(), self.res), 0) / self.res
        y = max(min(cursor.y(), self.res), 0) / self.res

        x, y = x * 2.0 - 1.0, y * 2.0 - 1.0
        y = -y
        rot = self.getActualWheelRotation()
        s = math.sin(rot)
        c = math.cos(rot)
        x, y = x * c - y * s, x * s + y * c

        cx, cy = settings.shape.getColorCoord(
            (x, y),
            (settings.ringThickness + settings.ringMargin) / (self.res / 2),
        )
        if settings.swapAxes:
            cx, cy = cy, cx
        if settings.reverseX:
            cx = 1 - cx
        if settings.reverseY:
            cy = 1 - cy
        STATE.updateVariableChannelsValue((cx, cy))
        self.update()

    def handleRingEdit(self, cursor: QVector2D):
        settings = STATE.currentSettings()
        x, y = cursor.x() / self.res * 2 - 1, cursor.y() / self.res * 2 - 1
        y = -y
        v = settings.shape.getRingValue((x, y), math.radians(settings.ringRotation))
        if settings.ringReversed:
            v = 1 - v
        STATE.updateLockedChannelValue(v)

    def handleMouse(self, event: QMouseEvent | None):
        if event == None or self.editStart == None:
            return

        cursor = self.editStart + (
            QVector2D(event.pos()) - self.editStart
        ) * computeMoveFactor(event)
        match self.editing:
            case ColorWheel.ColorWheelEditing.Wheel:
                self.handleWheelEdit(cursor)
            case ColorWheel.ColorWheelEditing.Ring:
                self.handleRingEdit(cursor)

    def mousePressEvent(self, a0: QMouseEvent | None):
        if a0 == None:
            return

        STATE.suppressColorSyncing = True
        settings = STATE.currentSettings()
        self.editStart = QVector2D(a0.pos())
        d = QVector2D(a0.pos()).distanceToPoint(QVector2D(self.res, self.res) * 0.5)
        if (
            settings.ringThickness == 0 and settings.ringMargin == 0
        ) or d < self.res * 0.5 - settings.ringThickness - settings.ringMargin:
            self.editing = ColorWheel.ColorWheelEditing.Wheel
        else:
            self.editing = ColorWheel.ColorWheelEditing.Ring

        self.handleMouse(a0)

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def mouseReleaseEvent(self, a0: QMouseEvent | None) -> None:
        STATE.suppressColorSyncing = False

    def enterEvent(self, a0: QEvent | None) -> None:
        STATE.suppressColorSyncing = True

    def leaveEvent(self, a0: QEvent | None) -> None:
        STATE.suppressColorSyncing = False

    def paintEvent(self, e: QPaintEvent | None):
        super().paintEvent(e)

        settings = STATE.currentSettings()
        ix, iy = 0, 0
        match STATE.lockedChannel:
            case 0:
                ix, iy = 1, 2
            case 1:
                ix, iy = 0, 2
            case 2:
                ix, iy = 0, 1

        cx, cy = STATE.color[ix], STATE.color[iy]
        if settings.reverseX:
            cx = 1 - cx
        if settings.reverseY:
            cy = 1 - cy
        if settings.swapAxes:
            cx, cy = cy, cx

        settings = STATE.currentSettings()
        x, y = settings.shape.getPos(
            (cx, cy),
            (settings.ringThickness + settings.ringMargin) / (self.res / 2),
        )

        rot = self.getActualWheelRotation()
        s = math.sin(-rot)
        c = math.cos(-rot)
        x, y = x * c - y * s, y * c + x * s

        x, y = x * 0.5 + 0.5, y * 0.5 + 0.5
        y = 1 - y
        x, y = x * self.res, y * self.res

        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))
        painter.drawArc(
            QRectF(x - 4, y - 3, 8, 8),
            0,
            360 * 16,
        )

        if settings.ringThickness == 0:
            return

        ringX, ringY = settings.shape.getRingPos(
            (
                1.0 - STATE.color[STATE.lockedChannel]
                if settings.ringReversed
                else STATE.color[STATE.lockedChannel]
            ),
            settings.ringThickness / (self.res / 2),
            math.radians(settings.ringRotation),
        )
        ringX, ringY = (ringX * 0.5 + 0.5) * self.res, (-ringY * 0.5 + 0.5) * self.res
        painter.drawArc(QRectF(ringX - 4, ringY - 2, 8, 8), 0, 360 * 16)

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
        settings = STATE.currentSettings()
        vert = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Vertex)
        vert.compileSourceCode(vertex)
        frag = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Fragment)
        fragCode = settings.shape.modifyShader(
            STATE.colorModel.modifyShader(wheelFragment)
        )

        if settings.ringThickness < 0.0 and settings.ringMargin < 0.0:
            begin = fragCode.find("BEGIN RING RENDERING")
            end = fragCode.find("END RING RENDERING")
            fragCode = fragCode[:begin] + fragCode[end:]

        if Application.activeWindow() == None:  # type: ignore
            return
        palette = Application.activeWindow().qwindow().palette()  # type: ignore
        bgColor: QColor = palette.color(QPalette.ColorRole.Window)
        fragCode = fragCode.replace(
            "const vec3 BACKGROUND_COLOR = vec3(0.0);",
            f"const vec3 BACKGROUND_COLOR = vec3({bgColor.redF()}, {bgColor.greenF()}, {bgColor.blueF()});",
        )

        frag.compileSourceCode(fragCode)
        self.program = QOpenGLShaderProgram(self.context())
        self.program.addShader(vert)
        self.program.addShader(frag)
        self.program.link()
        self.update()

    def getActualWheelRotation(self) -> float:
        settings = STATE.currentSettings()
        if settings.wheelRotateWithRing:
            c = STATE.color[STATE.lockedChannel]
            if settings.ringReversed:
                c = -c
            return (
                settings.rotation
                - (c + 0.5) * 2 * math.pi
                - math.radians(settings.ringRotation)
            )
        else:
            return settings.rotation

    def paintGL(self):
        if self.gl == None:
            QMessageBox.critical(
                self,
                "Extended Color Selector - Unable to get OpenGL Renderer\n",
                "This error is originated from ExtendedColorSelector. \n"
                "As we are using OpenGL for color wheel rendering, it is required to use OpenGL for rendering.\n"
                "Please goto Settings -> Configure Krita -> Display -> Canvas Acceleration -> "
                "Preferred Renderer, and select OpenGL. \n\n"
                "Krita WILL CRASH if you enter the canvas. So please change the settings in start menu.",
            )
            return

        if self.program == None:
            return

        self.program.bind()

        self.program.setUniformValue("res", int(self.res))
        self.program.setUniformValue(
            "constant", float(STATE.color[STATE.lockedChannel])
        )
        self.program.setUniformValue("constantPos", int(STATE.lockedChannel))

        settings = STATE.currentSettings()
        globalSettings = STATE.globalSettings

        axesConfig = 0
        if settings.swapAxes:
            axesConfig |= 1 << 0
        if settings.reverseX:
            axesConfig |= 1 << 1
        if settings.reverseY:
            axesConfig |= 1 << 2
        if settings.ringReversed:
            axesConfig |= 1 << 3

        self.program.setUniformValue("axesConfig", axesConfig)
        mn, mx = STATE.colorModel.limits()
        self.program.setUniformValue("lim_min", mn[0], mn[1], mn[2])
        self.program.setUniformValue("lim_max", mx[0], mx[1], mx[2])

        if globalSettings.outOfGamutColorEnabled:
            self.program.setUniformValue(
                "outOfGamut",
                globalSettings.outOfGamutColor[0],
                globalSettings.outOfGamutColor[1],
                globalSettings.outOfGamutColor[2],
            )
        else:
            self.program.setUniformValue("outOfGamut", -1.0, -1.0, -1.0)
        self.program.setUniformValue("rotation", self.getActualWheelRotation())
        self.program.setUniformValue("ringThickness", float(settings.ringThickness))
        self.program.setUniformValue("ringMargin", float(settings.ringMargin))
        self.program.setUniformValue(
            "ringRotation", float(math.radians(settings.ringRotation))
        )
        variables = None
        match STATE.lockedChannel:
            case 0:
                variables = STATE.color[1], STATE.color[2]
            case 1:
                variables = STATE.color[0], STATE.color[2]
            case 2:
                variables = STATE.color[0], STATE.color[1]
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
    def __init__(self, portable: bool):
        super().__init__()

        self.res = 1
        self.settings = SettingsPerColorModel(ColorModel.Rgb)
        self.globalSettings = GlobalSettings()
        self.editStart: float | None = None
        self.portable = portable
        self.compileShader()
        self.updateFromState()

        STATE.colorModelChanged.connect(self.compileShader)
        STATE.settingsChanged.connect(self.updateFromState)
        STATE.variablesChanged.connect(self.update)
        STATE.lockedChannelIndexChanged.connect(self.update)

    def updateFromState(self):
        globalSettings = STATE.globalSettings
        settings = STATE.currentSettings()
        barHeight, enabled = (
            (globalSettings.portableSelectorBarHeight, settings.enabled)
            if self.portable
            else (globalSettings.barHeight, settings.barEnabled)
        )

        self.setMinimumHeight(int(barHeight))
        if enabled:
            self.show()
        else:
            self.hide()
        self.update()

    def resizeEvent(self, e: QResizeEvent | None):
        super().resizeEvent(e)

        if e == None:
            return

        self.res = e.size().width()
        self.update()

    def handleMouse(self, event: QMouseEvent | None):
        if event == None or self.editStart == None:
            return

        x = self.editStart + (event.x() - self.editStart) * computeMoveFactor(event)
        x = max(min(x, self.res), 0)
        STATE.updateLockedChannelValue(x / self.res)
        self.update()

    def mousePressEvent(self, a0: QMouseEvent | None):
        if a0 == None:
            return

        self.editStart = a0.pos().x()
        self.handleMouse(a0)

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def mouseReleaseEvent(self, a0: QMouseEvent | None) -> None:
        STATE.suppressColorSyncing = False

    def enterEvent(self, a0: QEvent | None) -> None:
        STATE.suppressColorSyncing = True

    def leaveEvent(self, a0: QEvent | None) -> None:
        STATE.suppressColorSyncing = False

    def paintEvent(self, e: QPaintEvent | None):
        super().paintEvent(e)
        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))

        x = int(STATE.color[STATE.lockedChannel] * self.width())
        painter.drawRect(x - 1, 0, 2, self.height())

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
        settings = STATE.currentSettings()
        vert = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Vertex)
        vert.compileSourceCode(vertex)
        frag = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Fragment)
        frag.compileSourceCode(
            settings.shape.modifyShader(STATE.colorModel.modifyShader(barFragment))
        )
        self.program = QOpenGLShaderProgram(self.context())
        self.program.addShader(vert)
        self.program.addShader(frag)
        self.program.link()
        self.update()

    def paintGL(self):
        if self.gl == None:
            QMessageBox.critical(
                self,
                "Extended Color Selector - Unable to get OpenGL Renderer\n",
                "This error is originated from ExtendedColorSelector. \n"
                "As we are using OpenGL for color wheel rendering, it is required to use OpenGL for rendering.\n"
                "Please goto Settings -> Configure Krita -> Display -> Canvas Acceleration -> "
                "Preferred Renderer, and select OpenGL. \n\n"
                "Krita WILL CRASH if you enter the canvas. So please change the settings in start menu.",
            )
            return

        self.program.bind()

        ix, iy = 0, 0
        match STATE.lockedChannel:
            case 0:
                ix, iy = 1, 2
            case 1:
                ix, iy = 0, 2
            case 2:
                ix, iy = 0, 1

        self.program.setUniformValue("res", int(self.res))
        self.program.setUniformValue(
            "variables",
            float(STATE.color[ix]),
            float(STATE.color[iy]),
        )
        self.program.setUniformValue("constantPos", int(STATE.lockedChannel))
        mn, mx = STATE.colorModel.limits()
        self.program.setUniformValue("lim_min", mn[0], mn[1], mn[2])
        self.program.setUniformValue("lim_max", mx[0], mx[1], mx[2])

        if STATE.globalSettings.outOfGamutColorEnabled:
            self.program.setUniformValue(
                "outOfGamut",
                STATE.globalSettings.outOfGamutColor[0],
                STATE.globalSettings.outOfGamutColor[1],
                STATE.globalSettings.outOfGamutColor[2],
            )
        else:
            self.program.setUniformValue("outOfGamut", -1.0, -1.0, -1.0)

        gl = self.gl
        gl.glDrawArrays(gl.GL_TRIANGLE_STRIP, 0, 4)

    def resizeGL(self, w, h):
        if self.gl is None:
            return
        self.gl.glViewport(0, 0, w, h)
