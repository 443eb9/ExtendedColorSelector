from enum import Enum
from pathlib import Path

components: dict[str, str] = {}


class ColorSpace(Enum):
    Rgb = 0
    Hsv = 1
    Hsl = 2

    def getShaderComponent(self) -> str:
        name = self.shaderComponentName()
        if not name in components:
            components[name] = open(
                Path(__file__).parent
                / "shader_components"
                / "color_spaces"
                / f"{name}.glsl"
            ).read()[
                18:
            ]  # To strip the version directive

        return components[name]

    def shaderComponentName(self) -> str:
        match self:
            case ColorSpace.Rgb:
                return "rgb"
            case ColorSpace.Hsv:
                return "hsv"
            case ColorSpace.Hsl:
                return "hsl"

    def modifyShader(self, shader: str) -> str:
        component = self.getShaderComponent()
        return shader.replace("vec3 colorToRgb(vec3 color);", component)

    def displayName(self) -> str:
        match self:
            case ColorSpace.Rgb:
                return "RGB"
            case ColorSpace.Hsv:
                return "HSV"
            case ColorSpace.Hsl:
                return "HSL"

    def channels(self) -> list[str]:
        return list(self.displayName())

    def scales(self) -> tuple[float, float, float]:
        match self:
            case ColorSpace.Rgb:
                return 1, 1, 1
            case ColorSpace.Hsv:
                return 360, 1, 1
            case ColorSpace.Hsl:
                return 360, 1, 1
