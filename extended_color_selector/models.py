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

    def fromRgb(self, value: tuple[float, float, float]) -> tuple[float, float, float]:
        match self:
            case ColorSpace.Rgb:
                return value
            case ColorSpace.Hsv:
                return rgbToHsv(value)
            case ColorSpace.Hsl:
                return rgbToHsl(value)

    def toRgb(self, value: tuple[float, float, float]) -> tuple[float, float, float]:
        match self:
            case ColorSpace.Rgb:
                return value
            case ColorSpace.Hsv:
                return hsvToRgb(value)
            case ColorSpace.Hsl:
                return hslToRgb(value)


def colorSpaceFromKritaModel(model: str) -> ColorSpace | None:
    match model:
        case "RGBA":
            return ColorSpace.Rgb
        case _:
            return None


def rgbToHwb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    xMax = max(color)
    xMin = min(color)

    chroma = xMax - xMin

    hue = 0
    if chroma == 0:
        hue = 0
    elif color[0] == xMax:
        hue = 60 * (color[1] - color[2]) / chroma
    elif color[1] == xMax:
        hue = 60 * (2 + (color[2] - color[0]) / chroma)
    else:
        hue = 60 * (4 + (color[0] - color[1]) / chroma)
    hue = hue + 360 if hue < 0 else hue

    whiteness = xMin
    blackness = 1 - xMax
    return hue, whiteness, blackness


def rgbToHsv(color: tuple[float, float, float]) -> tuple[float, float, float]:
    h, w, b = rgbToHwb(color)
    v = 1 - b
    return h, 1 - (w / v) if v != 0 else 0, v


def rgbToHsl(color: tuple[float, float, float]) -> tuple[float, float, float]:
    h, s, v = rgbToHsv(color)
    l = v * (1 - s / 2)
    s = 0 if l == 0 or l == 1 else (v - l) / min(l, 1 - l)
    return h, s, l


def hsvToRgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    v = color[2]
    w = (1 - color[1]) * v

    h = color[0] / 60
    i = int(h)
    f = h - float(i)
    if i % 2 == 1:
        f = 1 - f

    n = w + f * (v - w)

    match i:
        case 0:
            return v, n, w
        case 1:
            return n, v, w
        case 2:
            return w, v, n
        case 3:
            return w, n, v
        case 4:
            return n, w, v
        case 5:
            return v, w, n
        case _:
            return 0, 0, 0


def hslToRgb(hsl: tuple[float, float, float]) -> tuple[float, float, float]:
    value = hsl[2] + hsl[1] * min(hsl[2], 1 - hsl[2])
    saturation = 0 if value == 0 else 2 * (1 - hsl[2] / value)
    color = hsl[0], saturation, value

    v = color[2]
    w = (1 - color[1]) * v

    h = color[1] / 60
    i = int(h)
    f = h - float(i)
    if i % 2 == 1:
        f = 1 - f

    n = w + f * (v - w)

    match i:
        case 0:
            return v, n, w
        case 1:
            return n, v, w
        case 2:
            return w, v, n
        case 3:
            return w, n, v
        case 4:
            return n, w, v
        case 5:
            return v, w, n
        case _:
            return 0, 0, 0
