from ctypes import CFUNCTYPE, c_int
from PyQt5.QtCore import QSize, QRectF, Qt, QPoint
from PyQt5.QtGui import (
    QMouseEvent,
    QPaintEvent,
    QResizeEvent,
    QOpenGLShader,
    QOpenGLShaderProgram,
    QPainter,
    QBrush,
    QColor,
    QVector2D,
    QPalette,
    QOpenGLContext,
)
from PyQt5.QtWidgets import (
    QOpenGLWidget,
    QMessageBox,
    QSizePolicy,
    QWidget,
    QDialog,
    QVBoxLayout,
    QFrame,
)
from pathlib import Path
from enum import IntEnum
import math

from .models import (
    ColorModel,
    SettingsPerColorModel,
    SettingsPerColorModel,
    GlobalSettings,
    transferColorModel,
)
from .internal_state import STATE
from .config import *
from .gamut_clipping import (
    getAxesLimitsInterpolated,
    mapAxesToLimited,
    unmapAxesFromLimited,
)


# Use [18:] to strip the version header, and add new one later before compiling
fullscreenVertex = open(Path(__file__).parent / "fullscreen.vert").read()[18:]
wheelFragment = open(Path(__file__).parent / "color_wheel.frag").read()[18:]
barFragment = open(Path(__file__).parent / "locked_channel_bar.frag").read()[18:]


def computeMoveFactor(e: QMouseEvent) -> float:
    m = e.modifiers()
    if m == Qt.KeyboardModifier.ShiftModifier:
        return 0.1
    if m == Qt.KeyboardModifier.AltModifier:
        return 0.01
    return 1.0


_funcTypes = {
    "glDrawArrays": CFUNCTYPE(None, c_int, c_int, c_int),
    "glViewport": CFUNCTYPE(None, c_int, c_int, c_int, c_int),
}


# From https://krita-artists.org/t/opengl-plugin-on-windows/92731/5
def getGLFunc(context: QOpenGLContext, name: str):
    sipPointer = context.getProcAddress(name.encode())
    funcType = _funcTypes[name]
    if sipPointer == None or funcType == None:
        raise Exception("Never happens")
    return funcType(int(sipPointer))


class ColorIndicatorBlocks(QDialog):
    def __init__(self) -> None:
        super().__init__()
        self.mainLayout = QVBoxLayout(self)
        self.setFixedSize(100, 150)
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.mainLayout.setSpacing(0)
        self.setWindowFlag(Qt.WindowType.FramelessWindowHint, True)
        self.setWindowFlag(Qt.WindowType.Tool, True)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)

        self.currentColorBox = QFrame()
        self.lastColorBox = QFrame()

        self.mainLayout.addWidget(self.currentColorBox, 1)
        self.mainLayout.addWidget(self.lastColorBox, 1)

        STATE.colorChanged.connect(self.updateColor)
        self.lastColor = STATE.color

    def updateColor(self):
        if self.isVisible():
            r, g, b = transferColorModel(STATE.color, STATE.colorModel, ColorModel.Rgb)
            r, g, b = (
                min(max(int(r * 256), 0), 255),
                min(max(int(g * 256), 0), 255),
                min(max(int(b * 256), 0), 255),
            )
            color = QColor(r, g, b)
            self.currentColorBox.setStyleSheet(f"background-color: {color.name()}")
        else:
            self.lastColor = STATE.color

    def popup(self, pos: QPoint):
        self.move(pos)
        r, g, b = transferColorModel(self.lastColor, STATE.colorModel, ColorModel.Rgb)
        r, g, b = (
            min(max(int(r * 256), 0), 255),
            min(max(int(g * 256), 0), 255),
            min(max(int(b * 256), 0), 255),
        )
        color = QColor(r, g, b)
        self.lastColorBox.setStyleSheet(f"background-color: {color.name()}")
        self.show()

    def shut(self):
        self.lastColor = STATE.color
        self.hide()


INDICATOR_BLOCKS = ColorIndicatorBlocks()


class OpenGLRenderer(QOpenGLWidget):
    class OpenGLWrapper:
        def __init__(self, context: QOpenGLContext) -> None:
            self.context = context
            self.glDrawArrays = getGLFunc(context, "glDrawArrays")
            self.glViewport = getGLFunc(context, "glViewport")
            self.GL_TRIANGLE_STRIP = 0x0005

        # From https://krita-artists.org/t/opengl-plugin-on-windows/92731/5
        def getVersionHeader(self):
            major, minor = self.context.format().version()
            if self.context.isOpenGLES():
                profile = "es"
                precision = "precision highp float;\n"
            else:
                profile = "core"
                precision = ""

            if OPENGL_VER_OVERRIDE_MAJOR != None:
                major = OPENGL_VER_OVERRIDE_MAJOR
            if OPENGL_VER_OVERRIDE_MINOR != None:
                minor = OPENGL_VER_OVERRIDE_MINOR
            if OPENGL_PROFILE_OVERRIDE != None:
                profile = OPENGL_PROFILE_OVERRIDE

            versionHeader = f"#version {major}{minor}0 {profile}\n{precision}"
            if OPENGL_INJECT_HEADERS != None:
                versionHeader = OPENGL_INJECT_HEADERS + versionHeader
            return versionHeader

    def __init__(self, parent: QWidget | None) -> None:
        super().__init__(parent)
        self.gl = None
        self.program = None
        self.fragment = ""
        self.vertex = ""

    def initializeGL(self) -> None:
        context = self.context()
        if context == None:
            QMessageBox.critical(
                self,
                "Extended Color Selector - Unable To Get OpenGL Context",
                "This shouldn't happen theoretically. Please open a issue on Github and provide your OS and hardware info.",
            )
            return
        self.program = QOpenGLShaderProgram(self.context())
        self.gl = OpenGLRenderer.OpenGLWrapper(context)

    def compileShader(self):
        if len(self.vertex) == 0 or len(self.fragment) == 0:
            return

        self.program = QOpenGLShaderProgram(self.context())
        vert = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Vertex)
        vert.compileSourceCode(self.vertex)
        frag = QOpenGLShader(QOpenGLShader.ShaderTypeBit.Fragment)
        frag.compileSourceCode(self.fragment)
        self.program.addShader(vert)
        self.program.addShader(frag)
        self.program.link()
        self.update()

    def resizeGL(self, w: int, h: int):
        if self.gl == None:
            return
        self.gl.glViewport(0, 0, w, h)


class ColorWheel(OpenGLRenderer):
    class ColorWheelEditing(IntEnum):
        Wheel = 1
        Ring = 2

    def __init__(self, indicatorBind: QWidget):
        super().__init__(None)
        self.indicatorBind = indicatorBind
        self.editing = ColorWheel.ColorWheelEditing.Wheel
        self.renderer = None
        self.setSizePolicy(
            QSizePolicy.Policy.MinimumExpanding, QSizePolicy.Policy.MinimumExpanding
        )

        self.res = 1
        self.editStart = QVector2D()
        self.shiftStart = QVector2D()
        self.setMinimumSize(MIN_WHEEL_SIZE, MIN_WHEEL_SIZE)
        self.setMaximumSize(MAX_WHEEL_SIZE, MAX_WHEEL_SIZE)

        STATE.settingsChanged.connect(self.updateShaders)
        STATE.colorModelChanged.connect(self.updateShaders)
        STATE.colorChanged.connect(self.update)
        STATE.lockedChannelIndexChanged.connect(self.update)

    def resizeEvent(self, e: QResizeEvent | None):
        if e == None:
            return

        size = min(e.size().width(), e.size().height())
        if e.size().width() != size or e.size().height() != size:
            self.resize(size, size)
            return

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

        ringThickness, ringMargin = self.getActualRingThicknessAndMargin()
        cx, cy = settings.shape.getColorCoord(
            (x, y),
            (ringThickness + ringMargin) / (self.res / 2),
        )

        if settings.reverseX:
            cx = 1 - cx
        if settings.reverseY:
            cy = 1 - cy
        if settings.swapAxes:
            cx, cy = cy, cx

        if settings.clipToSrgbGamut:
            cx, cy = mapAxesToLimited(
                STATE.colorModel,
                STATE.lockedChannel,
                STATE.lockedChannelValue(),
                (cx, cy),
            )

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
        if event == None:
            return

        f = computeMoveFactor(event)
        cursor = None
        if f == 1:
            cursor = self.editStart + (QVector2D(event.pos()) - self.editStart)
        else:
            cursor = self.editStart + (QVector2D(event.pos()) - self.shiftStart) * f
        match self.editing:
            case ColorWheel.ColorWheelEditing.Wheel:
                self.handleWheelEdit(cursor)
            case ColorWheel.ColorWheelEditing.Ring:
                self.handleRingEdit(cursor)

    def mousePressEvent(self, a0: QMouseEvent | None):
        if a0 == None:
            return

        settings = STATE.currentSettings()
        x, y = self.getCurrentWheelWidgetCoord()
        self.editStart = QVector2D(x, y)
        self.shiftStart = QVector2D(a0.pos())

        d = QVector2D(a0.pos()).distanceToPoint(QVector2D(self.res, self.res) * 0.5)
        ringThickness, ringMargin = self.getActualRingThicknessAndMargin()
        if (
            ringThickness == 0 and ringMargin == 0
        ) or d < self.res * 0.5 - ringThickness - ringMargin:
            self.editing = ColorWheel.ColorWheelEditing.Wheel
        else:
            self.editing = ColorWheel.ColorWheelEditing.Ring

        self.handleMouse(a0)

        INDICATOR_BLOCKS.popup(
            self.mapToGlobal(QPoint()) - QPoint(INDICATOR_BLOCKS.width(), 0)
        )

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def getActualRingThicknessAndMargin(self) -> tuple[float, float]:
        settings = STATE.currentSettings()
        if settings.ringEnabled and settings.ringThickness > 0.0:
            return settings.ringThickness, settings.ringMargin
        else:
            return 0.0, 0.0

    def getCurrentWheelWidgetCoord(self) -> tuple[float, float]:
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

        if settings.clipToSrgbGamut:
            cx, cy = unmapAxesFromLimited(
                STATE.colorModel,
                STATE.lockedChannel,
                STATE.lockedChannelValue(),
                (cx, cy),
            )

        if settings.swapAxes:
            cx, cy = cy, cx
        if settings.reverseX:
            cx = 1 - cx
        if settings.reverseY:
            cy = 1 - cy

        settings = STATE.currentSettings()
        ringThickness, ringMargin = self.getActualRingThicknessAndMargin()

        x, y = settings.shape.getPos(
            (cx, cy),
            (ringThickness + ringMargin) / (self.res / 2),
        )

        rot = self.getActualWheelRotation()
        s = math.sin(-rot)
        c = math.cos(-rot)
        x, y = x * c - y * s, y * c + x * s

        x, y = x * 0.5 + 0.5, y * 0.5 + 0.5
        y = 1 - y
        x, y = x * self.res, y * self.res

        return x, y

    def getCurrentRingWidgetCoord(self) -> tuple[float, float]:
        settings = STATE.currentSettings()

        ringThickness, _ = self.getActualRingThicknessAndMargin()

        ringX, ringY = settings.shape.getRingPos(
            (
                1.0 - STATE.color[STATE.lockedChannel]
                if settings.ringReversed
                else STATE.color[STATE.lockedChannel]
            ),
            ringThickness / (self.res / 2),
            math.radians(settings.ringRotation),
        )
        ringX, ringY = (ringX * 0.5 + 0.5) * self.res, (-ringY * 0.5 + 0.5) * self.res
        return ringX, ringY

    def paintEvent(self, e: QPaintEvent | None):
        super().paintEvent(e)

        x, y = self.getCurrentWheelWidgetCoord()
        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))
        painter.drawArc(
            QRectF(x - 4, y - 3, 8, 8),
            0,
            360 * 16,
        )

        ringThickness, _ = self.getActualRingThicknessAndMargin()
        if ringThickness == 0:
            return

        ringX, ringY = self.getCurrentRingWidgetCoord()
        painter.drawArc(QRectF(ringX - 4, ringY - 2, 8, 8), 0, 360 * 16)

    def initializeGL(self):
        super().initializeGL()
        self.updateShaders()

    def updateShaders(self):
        if self.gl == None or self.program == None:
            return

        settings = STATE.currentSettings()
        header = self.gl.getVersionHeader()
        self.vertex = header + fullscreenVertex

        fragCode = settings.shape.modifyShader(
            STATE.colorModel.modifyShader(wheelFragment)
        )

        ringThickness, _ = self.getActualRingThicknessAndMargin()
        if ringThickness < 0.0:
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
        self.fragment = header + fragCode

        self.compileShader()

    def getActualWheelRotation(self) -> float:
        settings = STATE.currentSettings()
        if settings.wheelRotateWithRing:
            c = STATE.color[STATE.lockedChannel]
            if settings.ringReversed:
                c = -c
            return (
                math.radians(settings.rotation)
                - (c + 0.5) * 2 * math.pi
                - math.radians(settings.ringRotation)
            )
        else:
            return math.radians(settings.rotation)

    def paintGL(self):
        if self.gl == None or self.program == None:
            return

        self.program.bind()

        self.program.setUniformValue("res", float(self.res))
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
        ringThickness, ringMargin = self.getActualRingThicknessAndMargin()
        self.program.setUniformValue("ringThickness", float(ringThickness))
        self.program.setUniformValue("ringMargin", float(ringMargin))
        self.program.setUniformValue(
            "ringRotation", float(math.radians(settings.ringRotation))
        )

        maybeColorfuledColor = (
            STATE.colorModel.makeColorful(STATE.color, STATE.lockedChannel)
            if settings.colorfulLockedChannel
            else STATE.color
        )
        variables = None
        match STATE.lockedChannel:
            case 0:
                variables = maybeColorfuledColor[1], maybeColorfuledColor[2]
            case 1:
                variables = maybeColorfuledColor[0], maybeColorfuledColor[2]
            case 2:
                variables = maybeColorfuledColor[0], maybeColorfuledColor[1]
            case _:
                return
        self.program.setUniformValue("variables", variables[0], variables[1])

        axesLimits = (
            getAxesLimitsInterpolated(
                STATE.colorModel, STATE.lockedChannel, STATE.lockedChannelValue()
            )
            if settings.clipToSrgbGamut
            else ((-1.0, -1.0), (-1.0, -1.0))
        )

        self.program.setUniformValue(
            "axesLimits",
            axesLimits[0][0],
            axesLimits[0][1],
            axesLimits[1][0],
            axesLimits[1][1],
        )

        self.gl.glDrawArrays(self.gl.GL_TRIANGLE_STRIP, 0, 4)


class LockedChannelBar(OpenGLRenderer):
    def __init__(self, portable: bool, indicatorBind: QWidget):
        super().__init__(None)

        self.res = 1
        self.settings = SettingsPerColorModel(ColorModel.Rgb)
        self.globalSettings = GlobalSettings()
        self.editStart = 0.0
        self.shiftStart = 0.0
        self.portable = portable
        self.indicatorBind = indicatorBind
        self.updateFromState()

        STATE.colorModelChanged.connect(self.updateShaders)
        STATE.settingsChanged.connect(self.updateFromState)
        STATE.colorChanged.connect(self.update)
        STATE.lockedChannelIndexChanged.connect(self.update)

    def updateFromState(self):
        globalSettings = STATE.globalSettings
        settings = STATE.currentSettings()
        barHeight, enabled = (
            (globalSettings.pBarHeight, settings.barEnabled)
            if self.portable
            else (globalSettings.barHeight, settings.barEnabled)
        )

        self.setFixedHeight(int(barHeight))
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
        if event == None:
            return

        f = computeMoveFactor(event)
        if f == 1:
            x = self.editStart + (event.x() - self.editStart)
        else:
            x = self.editStart + (event.x() - self.shiftStart) * f
        x = max(min(x, self.res), 0)
        STATE.updateLockedChannelValue(x / self.res)
        self.update()

    def mousePressEvent(self, a0: QMouseEvent | None):
        INDICATOR_BLOCKS.popup(
            self.mapToGlobal(QPoint()) - QPoint(INDICATOR_BLOCKS.width(), 0)
        )

        if a0 == None:
            return

        self.editStart = self.getCurrentWidgetCoord()
        self.shiftStart = a0.pos().x()
        self.handleMouse(a0)

    def mouseMoveEvent(self, a0: QMouseEvent | None):
        self.handleMouse(a0)

    def mouseReleaseEvent(self, a0: QMouseEvent | None) -> None:
        if a0 == None:
            return
        self.editStart = a0.pos().x()

    def getCurrentWidgetCoord(self) -> float:
        return STATE.color[STATE.lockedChannel] * self.width()

    def paintEvent(self, e: QPaintEvent | None):
        super().paintEvent(e)
        painter = QPainter(self)
        painter.setBrush(QBrush(QColor(255, 255, 255, 255)))

        x = int(self.getCurrentWidgetCoord())
        painter.drawRect(x - 1, 0, 2, self.height())

    def initializeGL(self):
        super().initializeGL()
        self.updateShaders()

    def updateShaders(self):
        if self.gl == None:
            return

        settings = STATE.currentSettings()
        header = self.gl.getVersionHeader()

        self.vertex = header + fullscreenVertex
        self.fragment = header + settings.shape.modifyShader(
            STATE.colorModel.modifyShader(barFragment)
        )
        self.program = QOpenGLShaderProgram(self.context())

        self.compileShader()

    def paintGL(self):
        if self.gl == None:
            return

        settings = STATE.currentSettings()
        self.program.bind()

        self.program.setUniformValue("res", float(self.res))
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

        maybeColorfuledColor = (
            STATE.colorModel.makeColorful(STATE.color, STATE.lockedChannel)
            if settings.colorfulLockedChannel
            else STATE.color
        )
        variables = None
        match STATE.lockedChannel:
            case 0:
                variables = maybeColorfuledColor[1], maybeColorfuledColor[2]
            case 1:
                variables = maybeColorfuledColor[0], maybeColorfuledColor[2]
            case 2:
                variables = maybeColorfuledColor[0], maybeColorfuledColor[1]
            case _:
                return
        self.program.setUniformValue("variables", variables[0], variables[1])

        self.program.setAttributeArray(
            0, [QVector2D(-1, -1), QVector2D(1, -1), QVector2D(-1, 1), QVector2D(1, 1)]
        )
        self.gl.glDrawArrays(self.gl.GL_TRIANGLE_STRIP, 0, 4)
