from enum import Enum
from pathlib import Path

components: dict[str, str] = {}


class ColorSpace(Enum):
    Rgb = 0

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

    def modifyShader(self, shader: str) -> str:
        component = self.getShaderComponent()
        start = shader.find("vec3 getColor(vec2 colorCoord, float constant, float constantPos);")
        shader.replace("vec3 getColor(vec2 colorCoord, float constant, float constantPos);", "")
        shader.replace("vec3 colorToRgb(vec3 color);", "")
        return shader[:start] + component + shader[start:]
