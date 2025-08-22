from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtGui import (
    QOpenGLVersionProfile,
    QResizeEvent,
    QSurfaceFormat,
    QOpenGLShader,
    QOpenGLShaderProgram,
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
                Path(__file__).parent / "shader_components" / "wheels" / f"{name}.glsl"
            ).read()[
                18:
            ]  # To strip the version directive

        return shader.replace(
            "vec2 getColorCoord(vec2 uv);",
            components[name],
        )


vertex = open(Path(__file__).parent / "fullscreen.vert").read()
fragment = open(Path(__file__).parent / "color_wheel.frag").read()


class ColorWheel(QOpenGLWidget):
    def __init__(self, parent: QWidget):
        super().__init__(parent)
        self.res = 1
        self.colorSpace = ColorSpace.Rgb
        self.shape = WheelShape.Square
        self.compileShader()
        self.constant = 0.0
        self.constantPos = 0

    def updateColorSpace(self, colorSpace: ColorSpace):
        self.colorSpace = colorSpace
        self.compileShader()

    def updateShape(self, shape: WheelShape):
        self.shape = shape
        self.compileShader()

    def resizeEvent(self, e: QResizeEvent | None):
        super().resizeEvent(e)

        if e == None:
            return

        size = min(e.size().width(), e.size().height())
        self.setFixedWidth(size)
        self.setFixedHeight(size)
        self.res = size

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
        frag.setProperty("SQUARE_WHEEL", True)

        fragCode = self.shape.modifyShader(self.colorSpace.modifyShader(fragment))
        print(fragCode)
        frag.compileSourceCode(fragCode)
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
        gl = self.gl
        gl.glDrawArrays(gl.GL_TRIANGLE_STRIP, 0, 4)

    def resizeGL(self, w, h):
        if self.gl == None:
            return
        self.gl.glViewport(0, 0, w, h)
