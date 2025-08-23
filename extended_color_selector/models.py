from enum import Enum
from pathlib import Path
import math

components: dict[str, str] = {}


class ColorModel(Enum):
    Rgb = 0
    Hsv = 1
    Hsl = 2
    Oklab = 3
    Xyz = 4

    def getShaderComponent(self) -> str:
        name = self.shaderComponentName()
        if not name in components:
            components[name] = open(
                Path(__file__).parent
                / "shader_components"
                / "color_models"
                / f"{name}.glsl"
            ).read()[
                18:
            ]  # To strip the version directive

        return components[name]

    def shaderComponentName(self) -> str:
        match self:
            case ColorModel.Rgb:
                return "rgb"
            case ColorModel.Hsv:
                return "hsv"
            case ColorModel.Hsl:
                return "hsl"
            case ColorModel.Oklab:
                return "oklab"
            case ColorModel.Xyz:
                return "xyz"

    def modifyShader(self, shader: str) -> str:
        component = self.getShaderComponent()
        return shader.replace("vec3 colorToRgb(vec3 color);", component)

    def displayName(self) -> str:
        match self:
            case ColorModel.Rgb:
                return "RGB"
            case ColorModel.Hsv:
                return "HSV"
            case ColorModel.Hsl:
                return "HSL"
            case ColorModel.Oklab:
                return "OkLab"
            case ColorModel.Xyz:
                return "XYZ"

    def channels(self) -> list[str]:
        match self:
            case ColorModel.Rgb:
                return ["R", "G", "B"]
            case ColorModel.Hsv:
                return ["H", "S", "V"]
            case ColorModel.Hsl:
                return ["H", "S", "L"]
            case ColorModel.Oklab:
                return ["L", "A", "B"]
            case ColorModel.Xyz:
                return ["X", "Y", "Z"]

    def limits(self) -> tuple[tuple[float, float, float], tuple[float, float, float]]:
        match self:
            case ColorModel.Rgb:
                return (0, 0, 0), (1, 1, 1)
            case ColorModel.Hsv:
                return (0, 0, 0), (360, 1, 1)
            case ColorModel.Hsl:
                return (0, 0, 0), (360, 1, 1)
            case ColorModel.Oklab:
                return (0, -1, -1), (1, 1, 1)
            case ColorModel.Xyz:
                return (0, 0, 0), (1, 1, 1)

    def normalize(
        self, color: tuple[float, float, float]
    ) -> tuple[float, float, float]:
        mn, mx = self.limits()
        return (
            (color[0] - mn[0]) / (mx[0] - mn[0]),
            (color[1] - mn[1]) / (mx[1] - mn[1]),
            (color[2] - mn[2]) / (mx[2] - mn[2]),
        )

    def unnormalize(
        self, color: tuple[float, float, float]
    ) -> tuple[float, float, float]:
        mn, mx = self.limits()
        return (
            color[0] * (mx[0] - mn[0]) + mn[0],
            color[1] * (mx[1] - mn[1]) + mn[1],
            color[2] * (mx[2] - mn[2]) + mn[2],
        )


def colorModelFromKrita(model: str) -> ColorModel | None:
    match model:
        case "RGBA":
            return ColorModel.Rgb
        case _:
            return None


def transferColorModel(
    color: tuple[float, float, float], fromSpace: ColorModel, toSpace: ColorModel
) -> tuple[float, float, float]:
    color = fromSpace.unnormalize(color)
    rgb = None
    match fromSpace:
        case ColorModel.Rgb:
            rgb = color
        case ColorModel.Hsv:
            rgb = hsvToRSrgb(color)
        case ColorModel.Hsl:
            rgb = hslToSrgb(color)
        case ColorModel.Oklab:
            rgb = oklabToSrgb(color)
        case ColorModel.Xyz:
            rgb = xyzToSrgb(color)

    unnormalized = None
    match toSpace:
        case ColorModel.Rgb:
            unnormalized = rgb
        case ColorModel.Hsv:
            unnormalized = srgbToHsv(rgb)
        case ColorModel.Hsl:
            unnormalized = srgbToHsl(rgb)
        case ColorModel.Oklab:
            unnormalized = oklabToSrgb(color)
        case ColorModel.Xyz:
            unnormalized = xyzToSrgb(color)

    return toSpace.normalize(unnormalized)


def srgbToLinear(color: tuple[float, float, float]) -> tuple[float, float, float]:
    def f(x: float) -> float:
        if x <= 0:
            return x
        if x <= 0.04045:
            return x / 12.92
        return (x + 0.055) / 1.055**2.4

    return f(color[0]), f(color[1]), f(color[2])


def linearToSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    def f(x: float) -> float:
        if x <= 0.0031308:
            return x * 12.92
        return 1.055 * x**-2.4 - 0.055

    return f(color[0]), f(color[1]), f(color[2])


def srgbToHwb(color: tuple[float, float, float]) -> tuple[float, float, float]:
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


def srgbToHsv(color: tuple[float, float, float]) -> tuple[float, float, float]:
    h, w, b = srgbToHwb(color)
    v = 1 - b
    return h, 1 - (w / v) if v != 0 else 0, v


def hsvToRSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
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


def hslToSrgb(hsl: tuple[float, float, float]) -> tuple[float, float, float]:
    value = hsl[2] + hsl[1] * min(hsl[2], 1 - hsl[2])
    saturation = 0 if value == 0 else 2 * (1 - hsl[2] / value)
    color = hsl[0], saturation, value

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


def srgbToHsl(color: tuple[float, float, float]) -> tuple[float, float, float]:
    h, s, v = srgbToHsv(color)
    l = v * (1 - s / 2)
    s = 0 if l == 0 or l == 1 else (v - l) / min(l, 1 - l)
    return h, s, l


def oklabToSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    lightness = color[0]
    a = color[1]
    b = color[2]

    l_ = lightness + 0.3963377774 * a + 0.2158037573 * b
    m_ = lightness - 0.1055613458 * a - 0.0638541728 * b
    s_ = lightness - 0.0894841775 * a - 1.2914855480 * b

    l = l_ * l_ * l_
    m = m_ * m_ * m_
    s = s_ * s_ * s_

    red = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s
    green = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s
    blue = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s

    return linearToSrgb((red, green, blue))


def srgbToOklab(color: tuple[float, float, float]) -> tuple[float, float, float]:
    red, green, blue = srgbToLinear(color)
    l = 0.4122214708 * red + 0.5363325363 * green + 0.0514459929 * blue
    m = 0.2119034982 * red + 0.6806995451 * green + 0.1073969566 * blue
    s = 0.0883024619 * red + 0.2817188376 * green + 0.6299787005 * blue
    l_ = l ** (1.0 / 3)
    m_ = m ** (1.0 / 3)
    s_ = s ** (1.0 / 3)
    l = 0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_
    a = 1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_
    b = 0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_
    return l, a, b


# print(srgbToLinear((0.2, 0.5, 0.8)))
# print(srgbToOklab((0.2, 0.5, 0.8)))
print(oklabToSrgb((0.76, 0.34, 0.38)))


def xyzToSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    x, y, z = color
    r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314
    g = x * -0.969266 + y * 1.8760108 + z * 0.041556
    b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252

    return linearToSrgb((r, g, b))


def srgbToXyz(color: tuple[float, float, float]) -> tuple[float, float, float]:
    r, g, b = srgbToLinear(color)

    x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375
    y = r * 0.2126729 + g * 0.7151522 + b * 0.072175
    z = r * 0.0193339 + g * 0.119192 + b * 0.9503041

    return x, y, z
