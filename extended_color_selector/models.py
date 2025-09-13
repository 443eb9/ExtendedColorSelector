from PyQt5.QtGui import QVector2D
from pathlib import Path
from enum import IntEnum
import math

from .config import DOCKER_NAME


class WheelShape(IntEnum):
    Square = 0
    Triangle = 1
    Circle = 2

    def displayName(self) -> str:
        match self:
            case WheelShape.Square:
                return "Square"
            case WheelShape.Triangle:
                return "Triangle"
            case WheelShape.Circle:
                return "Circle"

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
            "vec3 getColorCoordAndAntialias(vec2 p, float normalizedRingThickness);",
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
                halfA = d / math.sqrt(2.0) * 0.5
                x, y = min(max(x, -halfA), halfA), min(max(y, -halfA), halfA)

                return x / halfA * 0.5 + 0.5, y / halfA * 0.5 + 0.5
            case WheelShape.Triangle:
                p = QVector2D(x, y)
                RAD_120 = math.pi * 120.0 / 180.0
                t = 1.0 - normalizedRingThickness
                v0 = QVector2D(math.cos(RAD_120 * 0.0), math.sin(RAD_120 * 0.0)) * t
                v1 = QVector2D(math.cos(RAD_120 * 1.0), math.sin(RAD_120 * 1.0)) * t
                v2 = QVector2D(math.cos(RAD_120 * 2.0), math.sin(RAD_120 * 2.0)) * t
                vc = (v1 + v2) / 2.0
                vh = vc - v0
                a = (v0 - v1).length()
                h = max(vh.length(), 1e-6)

                y = QVector2D.dotProduct(p - v0, vh / h) / h
                b = p - (v0 * (1 - y) + v1 * y)
                x = b.length() / max(y * a, 1e-6)
                if QVector2D.dotProduct(b, v2 - v1) < 0.0:
                    x = 0

                x, y = min(max(x, 0.0), 1.0), min(max(y, 0.0), 1.0)

                return x, y
            case WheelShape.Circle:
                r = math.sqrt(x * x + y * y)
                r = min(r, 1 - normalizedRingThickness)
                a = math.atan2(y, x) / math.pi * 0.5 + 0.5
                return (r / (1 - normalizedRingThickness), a)

    def getRingValue(self, p: tuple[float, float], rotation: float) -> float:
        x = (math.atan2(p[1], p[0]) + rotation) / 2.0 / math.pi + 0.5
        return x - int(x)

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
                t = 1.0 - normalizedRingThickness
                v0 = QVector2D(math.cos(RAD_120 * 0.0), math.sin(RAD_120 * 0.0)) * t
                v1 = QVector2D(math.cos(RAD_120 * 1.0), math.sin(RAD_120 * 1.0)) * t
                v2 = QVector2D(math.cos(RAD_120 * 2.0), math.sin(RAD_120 * 2.0)) * t

                p = (v0 * (1 - y) + v1 * y) + (v2 - v1) * y * x
                return p.x(), p.y()
            case WheelShape.Circle:
                y *= 2 * math.pi
                y += math.pi
                x *= 1 - normalizedRingThickness
                x, y = math.cos(y) * x, math.sin(y) * x
                return x, y

    def getRingPos(
        self, value: float, normalizedRingThickness: float, rotation: float
    ) -> tuple[float, float]:
        value = (value + 0.5) * 2 * math.pi - rotation
        r = 1 - normalizedRingThickness * 0.5
        return math.cos(value) * r, math.sin(value) * r


class ColorModel(IntEnum):
    Rgb = 0
    Hsv = 1
    Hsl = 2
    Oklab = 3
    Xyz = 4
    Lab = 5
    Oklch = 6
    Okhsv = 7
    Okhsl = 8

    def modifyShader(self, shader: str) -> str:
        name = None
        match self:
            case ColorModel.Rgb:
                name = "rgb"
            case ColorModel.Hsv:
                name = "hsv"
            case ColorModel.Hsl:
                name = "hsl"
            case ColorModel.Oklab:
                name = "oklab"
            case ColorModel.Xyz:
                name = "xyz"
            case ColorModel.Lab:
                name = "lab"
            case ColorModel.Oklch:
                name = "oklch"
            case ColorModel.Okhsv:
                name = "okhsv"
            case ColorModel.Okhsl:
                name = "okhsl"

        component = open(
            Path(__file__).parent
            / "shader_components"
            / "color_models"
            / f"{name}.glsl"
        ).read()[
            18:
        ]  # To strip the version directive
        return shader.replace("vec3 colorToSrgb(vec3 color)", component)

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
            case ColorModel.Lab:
                return "Lab"
            case ColorModel.Oklch:
                return "OkLch"
            case ColorModel.Okhsv:
                return "OkHsv"
            case ColorModel.Okhsl:
                return "OkHsl"

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
            case ColorModel.Lab:
                return ["L", "A", "B"]
            case ColorModel.Oklch:
                return ["L", "C", "H"]
            case ColorModel.Okhsv:
                return ["H", "S", "V"]
            case ColorModel.Okhsl:
                return ["H", "S", "L"]

    # Returns the minimum and maximum values for each channel in their own space.
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
            case ColorModel.Lab:
                return (0, -1, -1), (1, 1, 1)
            case ColorModel.Oklch:
                return (0, 0, 0), (1, 1, 360)
            case ColorModel.Okhsv:
                return (0, 0, 0), (360, 100, 100)
            case ColorModel.Okhsl:
                return (0, 0, 0), (360, 100, 100)

    def toDisplayValues(
        self, color: tuple[float, float, float]
    ) -> tuple[float, float, float]:
        c0, c1, c2 = color
        match self:
            case ColorModel.Rgb:
                return c0 * 100, c1 * 100, c2 * 100
            case ColorModel.Hsv:
                return c0 * 360, c1 * 100, c2 * 100
            case ColorModel.Hsl:
                return c0 * 360, c1 * 100, c2 * 100
            case ColorModel.Oklab:
                return c0 * 100, (c1 - 0.5) * 200, (c2 - 0.5) * 200
            case ColorModel.Xyz:
                return c0 * 100, c1 * 100, c2 * 100
            case ColorModel.Lab:
                return c0 * 100, (c1 - 0.5) * 200, (c2 - 0.5) * 200
            case ColorModel.Oklch:
                return c0 * 100, c1 * 100, c2 * 360
            case ColorModel.Okhsv:
                return c0 * 360, c1 * 100, c2 * 100
            case ColorModel.Okhsl:
                return c0 * 360, c1 * 100, c2 * 100

    def fromDisplayValues(
        self, displayed: tuple[float, float, float]
    ) -> tuple[float, float, float]:
        c0, c1, c2 = displayed
        match self:
            case ColorModel.Rgb:
                return c0 / 100, c1 / 100, c2 / 100
            case ColorModel.Hsv:
                return c0 / 360, c1 / 100, c2 / 100
            case ColorModel.Hsl:
                return c0 / 360, c1 / 100, c2 / 100
            case ColorModel.Oklab:
                return c0 / 100, (c1 + 0.5) / 200, (c2 + 0.5) / 200
            case ColorModel.Xyz:
                return c0 / 100, c1 / 100, c2 / 100
            case ColorModel.Lab:
                return c0 / 100, (c1 + 0.5) / 200, (c2 + 0.5) / 200
            case ColorModel.Oklch:
                return c0 / 100, c1 / 100, c2 / 360
            case ColorModel.Okhsv:
                return c0 / 360, c1 / 100, c2 / 100
            case ColorModel.Okhsl:
                return c0 / 360, c1 / 100, c2 / 100

    def displayLimits(
        self,
    ) -> tuple[tuple[float, float, float], tuple[float, float, float]]:
        match self:
            case ColorModel.Rgb:
                return (0, 0, 0), (100, 100, 100)
            case ColorModel.Hsv:
                return (0, 0, 0), (360, 100, 100)
            case ColorModel.Hsl:
                return (0, 0, 0), (360, 100, 100)
            case ColorModel.Oklab:
                return (0, -100, -100), (100, 100, 100)
            case ColorModel.Xyz:
                return (0, 0, 0), (100, 100, 100)
            case ColorModel.Lab:
                return (0, -100, -100), (100, 100, 100)
            case ColorModel.Oklch:
                return (0, 0, 0), (100, 100, 360)
            case ColorModel.Okhsv:
                return (0, 0, 0), (360, 100, 100)
            case ColorModel.Okhsl:
                return (0, 0, 0), (360, 100, 100)

    def clamp(self, color: tuple[float, float, float]) -> tuple[float, float, float]:
        mn, mx = self.limits()
        return (
            max(min(color[0], mx[0]), mn[0]),
            max(min(color[1], mx[1]), mn[1]),
            max(min(color[2], mx[2]), mn[2]),
        )

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

    def makeColorful(
        self, color: tuple[float, float, float], channel: int
    ) -> tuple[float, float, float]:
        ch = color[channel]

        match self:
            case ColorModel.Rgb:
                return color
            case ColorModel.Hsv:
                match channel:
                    case 0:
                        return ch, 1, 1
                    case _:
                        return color
            case ColorModel.Hsl:
                match channel:
                    case 0:
                        return ch, 1, 0.5
                    case _:
                        return color
            case ColorModel.Oklab:
                return color
            case ColorModel.Xyz:
                return color
            case ColorModel.Lab:
                return color
            case ColorModel.Oklch:
                # TODO maybe?
                return color
            case ColorModel.Okhsv:
                match channel:
                    case 0:
                        return ch, 1, 1
                    case _:
                        return color
            case ColorModel.Okhsl:
                match channel:
                    case 0:
                        return ch, 1, 0.5
                    case _:
                        return color

    def isNotSrgbBased(self) -> bool:
        return self in [
            ColorModel.Oklab,
            ColorModel.Xyz,
            ColorModel.Lab,
            ColorModel.Oklch,
        ]

    def isColorfulable(self) -> bool:
        return self in [
            ColorModel.Hsv,
            ColorModel.Hsl,
            ColorModel.Okhsv,
            ColorModel.Okhsl,
        ]


def colorModelFromKrita(model: str) -> ColorModel | None:
    match model:
        case "RGBA":
            return ColorModel.Rgb
        case "LABA":
            return ColorModel.Lab
        case "XYZA":
            return ColorModel.Xyz
        case _:
            return None


# Reference color is a color represented using `toModel` color model.
# It's used to eliminate some indeterminable values.
# For example, when input color is RGB [0, 0, 0], and we need to convert
# it into HSV, then the hue is indeterminable. In this case, we will use
# the hue in reference color.
def transferColorModel(
    color: tuple[float, float, float],
    fromModel: ColorModel,
    toModel: ColorModel,
    referenceColor: tuple[float, float, float] | None = None,
    clamp: bool = True,
) -> tuple[float, float, float]:
    if fromModel == toModel:
        return color

    color = fromModel.unnormalize(color)

    xyz = None
    match fromModel:
        case ColorModel.Rgb:
            xyz = srgbToXyz(color)
        case ColorModel.Hsv:
            xyz = srgbToXyz(hsvToSrgb(color))
        case ColorModel.Hsl:
            xyz = srgbToXyz(hslToSrgb(color))
        case ColorModel.Oklab:
            xyz = oklabToXyz(color)
        case ColorModel.Xyz:
            xyz = color
        case ColorModel.Lab:
            xyz = labToXyz(color)
        case ColorModel.Oklch:
            xyz = oklchToXyz(color)
        case ColorModel.Okhsv:
            xyz = srgbToXyz(okhsvToSrgb(color))
        case ColorModel.Okhsl:
            xyz = srgbToXyz(okhslToSrgb(color))

    unnormalized = None
    match toModel:
        case ColorModel.Rgb:
            unnormalized = xyzToSrgb(xyz)
        case ColorModel.Hsv:
            unnormalized = srgbToHsv(xyzToSrgb(xyz))
        case ColorModel.Hsl:
            unnormalized = srgbToHsl(xyzToSrgb(xyz))
        case ColorModel.Oklab:
            unnormalized = xyzToOklab(xyz)
        case ColorModel.Xyz:
            unnormalized = xyz
        case ColorModel.Lab:
            unnormalized = xyzToLab(xyz)
        case ColorModel.Oklch:
            unnormalized = xyzToOklch(xyz)
        case ColorModel.Okhsv:
            unnormalized = srgbToOkhsv(xyzToSrgb(xyz))
        case ColorModel.Okhsl:
            unnormalized = srgbToOkhsl(xyzToSrgb(xyz))

    c0, c1, c2 = toModel.normalize(unnormalized)

    if referenceColor != None:
        rc0, rc1, rc2 = referenceColor

        # Eliminate indetermination
        match toModel:
            case ColorModel.Rgb:
                pass
            case ColorModel.Hsv:
                c0 = c0 if c1 > 1e-4 else rc0
                c0, c1 = (c0, c1) if c2 > 1e-4 else (rc0, rc1)
            case ColorModel.Hsl:
                c0 = c0 if c1 > 1e-4 else rc0
                c0, c1 = (c0, c1) if (c2 > 1e-4 and c2 < 1 - 1e-4) else (rc0, rc1)
            case ColorModel.Oklab:
                c1, c2 = (c1, c2) if (c0 > 1e-4 and c0 < 1 - 1e-4) else (rc1, rc2)
            case ColorModel.Xyz:
                pass
            case ColorModel.Lab:
                c1, c2 = (c1, c2) if (c0 > 1e-4 and c0 < 1 - 1e-4) else (rc1, rc2)
            case ColorModel.Oklch:
                c0 = c0 if (c1 > 1e-4) else rc0
                c1, c2 = (c1, c2) if (c0 > 1e-4 and c0 < 1 - 1e-4) else (rc1, rc2)
            case ColorModel.Okhsv:
                c0 = c0 if c1 > 1e-4 else rc0
                c0, c1 = (c0, c1) if c2 > 1e-4 else (rc0, rc1)

    result = c0, c1, c2

    if clamp:
        result = (
            min(max(result[0], 0.0), 1.0),
            min(max(result[1], 0.0), 1.0),
            min(max(result[2], 0.0), 1.0),
        )
    return result


def cbrt(x: float) -> float:
    return x ** (1.0 / 3) if x > 0 else -((-x) ** (1.0 / 3))


# The following color model conversion code is translated from `bevy` under MIT license
#
# Original license:
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

LAB_CIE_EPSILON = 216.0 / 24389.0
LAB_CIE_KAPPA = 24389.0 / 27.0
XYZ_D65_WHITE = 0.95047, 1.0, 1.08883


def srgbToLinear(color: tuple[float, float, float]) -> tuple[float, float, float]:
    def f(x: float) -> float:
        if x <= 0:
            return x
        if x <= 0.04045:
            return x / 12.92
        return ((x + 0.055) / 1.055) ** 2.4

    return f(color[0]), f(color[1]), f(color[2])


def linearToSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    def f(x: float) -> float:
        if x <= 0.0031308:
            return x * 12.92
        return 1.055 * (x ** (1 / 2.4)) - 0.055

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


def hsvToSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
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
            return v, n, w


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
            return v, n, w


def srgbToHsl(color: tuple[float, float, float]) -> tuple[float, float, float]:
    h, s, v = srgbToHsv(color)
    l = v * (1 - s / 2)
    s = 0 if l == 0 or l == 1 else (v - l) / min(l, 1 - l)
    return h, s, l


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


def labToXyz(color: tuple[float, float, float]) -> tuple[float, float, float]:
    l = 100.0 * color[0]
    a = 100.0 * color[1]
    b = 100.0 * color[2]

    fy = (l + 16.0) / 116.0
    fx = a / 500.0 + fy
    fz = fy - b / 200.0

    fx3 = fx * fx * fx
    xr = fx3 if fx3 > LAB_CIE_EPSILON else (116.0 * fx - 16.0) / LAB_CIE_KAPPA

    yr = (
        ((l + 16.0) / 116.0) ** 3
        if l > LAB_CIE_EPSILON * LAB_CIE_KAPPA
        else l / LAB_CIE_KAPPA
    )

    fz3 = fz * fz * fz
    zr = fz3 if fz3 > LAB_CIE_EPSILON else (116.0 * fz - 16.0) / LAB_CIE_KAPPA

    x = xr * XYZ_D65_WHITE[0]
    y = yr * XYZ_D65_WHITE[1]
    z = zr * XYZ_D65_WHITE[2]
    return x, y, z


def xyzToLab(color: tuple[float, float, float]) -> tuple[float, float, float]:
    x, y, z = color

    xr = x / XYZ_D65_WHITE[0]
    yr = y / XYZ_D65_WHITE[1]
    zr = z / XYZ_D65_WHITE[2]
    fx = cbrt(xr) if xr > LAB_CIE_EPSILON else (LAB_CIE_KAPPA * xr + 16.0) / 116.0
    fy = cbrt(yr) if yr > LAB_CIE_EPSILON else (LAB_CIE_KAPPA * yr + 16.0) / 116.0
    fz = cbrt(zr) if zr > LAB_CIE_EPSILON else (LAB_CIE_KAPPA * zr + 16.0) / 116.0
    l = 1.16 * fy - 0.16
    a = 5.00 * (fx - fy)
    b = 2.00 * (fy - fz)

    return l, a, b


def oklchToXyz(color: tuple[float, float, float]) -> tuple[float, float, float]:
    l = color[0]
    hue = math.radians(color[2])
    a = color[1] * math.cos(hue)
    b = color[1] * math.sin(hue)

    return oklabToXyz((l, a, b))


def xyzToOklch(color: tuple[float, float, float]) -> tuple[float, float, float]:
    l, a, b = xyzToOklab(color)
    chroma = math.hypot(a, b)
    hue = math.degrees(math.atan2(b, a))

    hue = hue + 360.0 if hue < 0.0 else hue

    return l, chroma, hue


# -----------------------
# End bevy code reference
# -----------------------


# https:#bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
def xyzToOklab(color: tuple[float, float, float]) -> tuple[float, float, float]:
    x = color[0]
    y = color[1]
    z = color[2]

    l_ = 0.8189330101 * x + 0.3618667424 * y - 0.1288597137 * z
    m_ = 0.0329845436 * x + 0.9293118715 * y + 0.0361456387 * z
    s_ = 0.0482003018 * x + 0.2643662691 * y + 0.6338517070 * z

    l_ = cbrt(l_)
    m_ = cbrt(m_)
    s_ = cbrt(s_)

    l = 0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_
    a = 1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_
    b = 0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_

    return toe(l), a, b


# https:#bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
# Inverse matrices are computed from the matrix in the post
def oklabToXyz(color: tuple[float, float, float]) -> tuple[float, float, float]:
    l = toeInv(color[0])
    a = color[1]
    b = color[2]

    l_ = 0.9999999984 * l + 0.3963377921 * a + 0.2158037580 * b
    m_ = 1.0000000088 * l - 0.10556134232 * a - 0.0638541747 * b
    s_ = 1.0000000546 * l - 0.08948418209 * a - 1.2914855378 * b

    l_ = l_**3
    m_ = m_**3
    s_ = s_**3

    x = +1.2270138511 * l_ - 0.5577999806 * m_ + 0.2812561489 * s_
    y = -0.0405801784 * l_ + 1.1122568696 * m_ - 0.0716766786 * s_
    z = -0.0763812845 * l_ - 0.4214819784 * m_ + 1.5861632204 * s_

    return x, y, z


# https://bottosson.github.io/posts/colorpicker/
# Copyright (c) 2021 Björn Ottosson

# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


def linearSrgbToOklab(color: tuple[float, float, float]) -> tuple[float, float, float]:
    r, g, b = color

    l = 0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b
    m = 0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b
    s = 0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b

    l_ = cbrt(l)
    m_ = cbrt(m)
    s_ = cbrt(s)

    return (
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_,
    )


def oklabToLinearSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    l, a, b = color

    l_ = l + 0.3963377774 * a + 0.2158037573 * b
    m_ = l - 0.1055613458 * a - 0.0638541728 * b
    s_ = l - 0.0894841775 * a - 1.2914855480 * b

    l = l_ * l_ * l_
    m = m_ * m_ * m_
    s = s_ * s_ * s_

    return (
        +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s,
        -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s,
        -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s,
    )


# Finds the maximum saturation possible for a given hue that fits in sRGB
# Saturation here is defined as S = C/L
# a and b must be normalized so a^2 + b^2 == 1
def computeMaxSaturation(a: float, b: float) -> float:
    # Max saturation will be when one of r, g or b goes below zero.

    # Select different coefficients depending on which component goes below zero first
    k0 = None
    k1 = None
    k2 = None
    k3 = None
    k4 = None
    wl = None
    wm = None
    ws = None

    if -1.88170328 * a - 0.80936493 * b > 1:
        # Red component
        k0 = +1.19086277
        k1 = +1.76576728
        k2 = +0.59662641
        k3 = +0.75515197
        k4 = +0.56771245
        wl = +4.0767416621
        wm = -3.3077115913
        ws = +0.2309699292
    elif 1.81444104 * a - 1.19445276 * b > 1:
        # Green component
        k0 = +0.73956515
        k1 = -0.45954404
        k2 = +0.08285427
        k3 = +0.12541070
        k4 = +0.14503204
        wl = -1.2684380046
        wm = +2.6097574011
        ws = -0.3413193965
    else:
        # Blue component
        k0 = +1.35733652
        k1 = -0.00915799
        k2 = -1.15130210
        k3 = -0.50559606
        k4 = +0.00692167
        wl = -0.0041960863
        wm = -0.7034186147
        ws = +1.7076147010

    # Approximate max saturation using a polynomial:

    S = k0 + k1 * a + k2 * b + k3 * a * a + k4 * a * b

    # Do one step Halley's method to get closer
    # this gives an error less than 10e6, except for some blue hues where the dS/dh is close to infinite
    # this should be sufficient for most applications, otherwise do two/three steps

    k_l = +0.3963377774 * a + 0.2158037573 * b
    k_m = -0.1055613458 * a - 0.0638541728 * b
    k_s = -0.0894841775 * a - 1.2914855480 * b

    l_ = 1.0 + S * k_l
    m_ = 1.0 + S * k_m
    s_ = 1.0 + S * k_s

    l = l_ * l_ * l_
    m = m_ * m_ * m_
    s = s_ * s_ * s_

    l_dS = 3.0 * k_l * l_ * l_
    m_dS = 3.0 * k_m * m_ * m_
    s_dS = 3.0 * k_s * s_ * s_

    l_dS2 = 6.0 * k_l * k_l * l_
    m_dS2 = 6.0 * k_m * k_m * m_
    s_dS2 = 6.0 * k_s * k_s * s_

    f = wl * l + wm * m + ws * s
    f1 = wl * l_dS + wm * m_dS + ws * s_dS
    f2 = wl * l_dS2 + wm * m_dS2 + ws * s_dS2

    S = S - f * f1 / (f1 * f1 - 0.5 * f * f2)

    return S


# finds L_cusp and C_cusp for a given hue
# a and b must be normalized so a^2 + b^2 == 1
def findCusp(a, b) -> tuple[float, float]:
    # First, find the maximum saturation (saturation S = C/L)
    S_cusp = computeMaxSaturation(a, b)

    # Convert to linear sRGB to find the first point where at least one of r,g or b >= 1:
    rgb_at_max = oklabToLinearSrgb((1, S_cusp * a, S_cusp * b))
    L_cusp = cbrt(1.0 / max(rgb_at_max))
    C_cusp = L_cusp * S_cusp

    return L_cusp, C_cusp


# Finds intersection of the line defined by
# L = L0 * (1 - t) + t * L1
# C = t * C1
# a and b must be normalized so a^2 + b^2 == 1
def findGamutIntersection(a: float, b: float, L1: float, C1: float, L0: float) -> float:
    # Find the cusp of the gamut triangle
    Lcusp, Ccusp = findCusp(a, b)

    # Find the intersection for upper and lower half seprately
    t = None
    if ((L1 - L0) * Ccusp - (Lcusp - L0) * C1) <= 0.0:
        # Lower half
        t = Ccusp * L0 / (C1 * Lcusp + Ccusp * (L0 - L1))

    else:
        # Upper half

        # First intersect with triangle
        t = Ccusp * (L0 - 1.0) / (C1 * (Lcusp - 1.0) + Ccusp * (L0 - L1))

        # Then one step Halley's method
        dL = L1 - L0
        dC = C1

        k_l = 0.3963377774 * a + 0.2158037573 * b
        k_m = -0.1055613458 * a - 0.0638541728 * b
        k_s = -0.0894841775 * a - 1.2914855480 * b

        l_dt = dL + dC * k_l
        m_dt = dL + dC * k_m
        s_dt = dL + dC * k_s

        # If higher accuracy is required, 2 or 3 iterations of the following block can be used:
        L = L0 * (1.0 - t) + t * L1
        C = t * C1

        l_ = L + C * k_l
        m_ = L + C * k_m
        s_ = L + C * k_s

        l = l_ * l_ * l_
        m = m_ * m_ * m_
        s = s_ * s_ * s_

        ldt = 3 * l_dt * l_ * l_
        mdt = 3 * m_dt * m_ * m_
        sdt = 3 * s_dt * s_ * s_

        ldt2 = 6 * l_dt * l_dt * l_
        mdt2 = 6 * m_dt * m_dt * m_
        sdt2 = 6 * s_dt * s_dt * s_

        r = 4.07674166210 * l - 3.30771159130 * m + 0.23096992920 * s - 1
        r1 = 4.07674166210 * ldt - 3.30771159130 * mdt + 0.23096992920 * sdt
        r2 = 4.07674166210 * ldt2 - 3.30771159130 * mdt2 + 0.23096992920 * sdt2

        u_r = r1 / (r1 * r1 - 0.50 * r * r2)
        t_r = -r * u_r

        g = -1.26843800460 * l + 2.60975740110 * m - 0.34131939650 * s - 1
        g1 = -1.26843800460 * ldt + 2.60975740110 * mdt - 0.34131939650 * sdt
        g2 = -1.26843800460 * ldt2 + 2.60975740110 * mdt2 - 0.34131939650 * sdt2

        u_g = g1 / (g1 * g1 - 0.50 * g * g2)
        t_g = -g * u_g

        b = -0.00419608630 * l - 0.70341861470 * m + 1.70761470100 * s - 1
        b1 = -0.00419608630 * ldt - 0.70341861470 * mdt + 1.70761470100 * sdt
        b2 = -0.00419608630 * ldt2 - 0.70341861470 * mdt2 + 1.70761470100 * sdt2

        u_b = b1 / (b1 * b1 - 0.5 * b * b2)
        t_b = -b * u_b

        t_r = t_r if u_r >= 0.0 else math.inf
        t_g = t_g if u_g >= 0.0 else math.inf
        t_b = t_b if u_b >= 0.0 else math.inf

        t += min(t_r, min(t_g, t_b))

    return t


K1 = 0.206
K2 = 0.03
K3 = (1.0 + K1) / (1.0 + K2)


def toe(x: float) -> float:
    return 0.5 * (
        K3 * x - K1 + ((K3 * x - K1) * (K3 * x - K1) + 4 * K2 * K3 * x) ** 0.5
    )


# inverse toe function for L_r
def toeInv(x: float):
    return (x * x + K1 * x) / (K3 * (x + K2))


def toST(L_cusp: float, C_cusp: float) -> tuple[float, float]:
    return C_cusp / L_cusp, C_cusp / (1 - L_cusp)


def okhsvToSrgb(color: tuple[float, float, float]) -> tuple[float, float, float]:
    h = color[0] / 360
    s = max(color[1], 1e-5) / 100
    v = max(color[2], 1e-5) / 100

    a_ = math.cos(2.0 * math.pi * h)
    b_ = math.sin(2.0 * math.pi * h)

    L_cusp, C_cusp = findCusp(a_, b_)
    S_max, T_max = toST(L_cusp, C_cusp)
    S_0 = 0.5
    k = 1 - S_0 / S_max

    # first we compute L and V as if the gamut is a perfect triangle:

    # L, C when v==1:
    L_v = 1 - s * S_0 / (S_0 + T_max - T_max * k * s)
    C_v = s * T_max * S_0 / (S_0 + T_max - T_max * k * s)

    L = v * L_v
    C = v * C_v

    # then we compensate for both toe and the curved top part of the triangle:
    L_vt = toeInv(L_v)
    C_vt = C_v * L_vt / L_v

    L_new = toeInv(L)
    C = C * L_new / L
    L = L_new

    rgb_scale = oklabToLinearSrgb((L_vt, a_ * C_vt, b_ * C_vt))
    scale_L = cbrt(1.0 / max(max(rgb_scale), 1e-5))

    L = L * scale_L
    C = C * scale_L

    rgb = oklabToLinearSrgb((L, C * a_, C * b_))
    return linearToSrgb(rgb)


def srgbToOkhsv(rgb: tuple[float, float, float]) -> tuple[float, float, float]:
    rgb = max(rgb[0], 1e-5), max(rgb[1], 1e-5), max(rgb[2], 1e-5)
    lab = linearSrgbToOklab(srgbToLinear(rgb))

    C = (lab[1] * lab[1] + lab[2] * lab[2]) ** 0.5
    a_ = lab[1] / C
    b_ = lab[2] / C

    L = lab[0]
    h = 0.5 + 0.5 * math.atan2(-lab[2], -lab[1]) / math.pi

    cusp = findCusp(a_, b_)
    S_max, T_max = toST(cusp[0], cusp[1])
    S_0 = 0.5
    k = 1 - S_0 / S_max

    # first we find L_v, C_v, L_vt and C_vt

    t = T_max / (C + L * T_max)
    L_v = t * L
    C_v = t * C

    L_vt = toeInv(L_v)
    C_vt = C_v * L_vt / L_v

    # we can then use these to invert the step that compensates for the toe and the curved top part of the triangle:
    rgb_scale = oklabToLinearSrgb((L_vt, a_ * C_vt, b_ * C_vt))
    scale_L = cbrt(1.0 / max(max(rgb_scale), 1e-5))

    L = L / scale_L
    C = C / scale_L

    C = C * toe(L) / L
    L = toe(L)

    # we can now compute v and s:

    v = L / L_v
    s = (S_0 + T_max) * C_v / ((T_max * S_0) + T_max * k * C_v)

    return h * 360, s * 100, v * 100


# Returns a smooth approximation of the location of the cusp
# This polynomial was created by an optimization process
# It has been designed so that S_mid < S_max and T_mid < T_max
def get_ST_mid(a_: float, b_: float) -> tuple[float, float]:
    S = 0.11516993 + 1 / (
        +7.44778970
        + 4.15901240 * b_
        + a_
        * (
            -2.19557347
            + 1.75198401 * b_
            + a_
            * (
                -2.13704948
                - 10.02301043 * b_
                + a_ * (-4.24894561 + 5.38770819 * b_ + 4.69891013 * a_)
            )
        )
    )

    T = 0.11239642 + 1.0 / (
        +1.61320320
        - 0.68124379 * b_
        + a_
        * (
            +0.40370612
            + 0.90148123 * b_
            + a_
            * (
                -0.27087943
                + 0.61223990 * b_
                + a_ * (+0.00299215 - 0.45399568 * b_ - 0.14661872 * a_)
            )
        )
    )

    return S, T


def get_Cs(L: float, a_: float, b_: float) -> tuple[float, float, float]:
    cusp = findCusp(a_, b_)

    C_max = findGamutIntersection(a_, b_, L, 1, L)
    ST_max = toST(cusp[0], cusp[1])

    # Scale factor to compensate for the curved part of gamut shape:
    k = C_max / min((L * ST_max[0]), (1 - L) * ST_max[1])

    C_mid = 0.0

    ST_mid = get_ST_mid(a_, b_)

    # Use a soft minimum function, instead of a sharp triangle shape to get a smooth value for chroma.
    C_a = L * ST_mid[0]
    C_b = (1 - L) * ST_mid[1]
    C_mid = (
        0.9
        * k
        * math.sqrt(
            math.sqrt(1 / (1 / (C_a * C_a * C_a * C_a) + 1 / (C_b * C_b * C_b * C_b)))
        )
    )

    # for C_0, the shape is independent of hue, so ST are constant. Values picked to roughly be the average values of ST.
    C_a = L * 0.4
    C_b = (1 - L) * 0.8

    # Use a soft minimum function, instead of a sharp triangle shape to get a smooth value for chroma.
    C_0 = math.sqrt(1 / (1 / (C_a * C_a) + 1 / (C_b * C_b)))

    return C_0, C_mid, C_max


def okhslToSrgb(hsl: tuple[float, float, float]) -> tuple[float, float, float]:
    h, s, l = hsl[0] / 360, max(hsl[1], 1e-5) / 100, max(hsl[2], 1e-5) / 100

    if l == 1.0:
        return 1, 1, 1

    elif l == 0:
        return 0, 0, 0

    a_ = math.cos(2 * math.pi * h)
    b_ = math.sin(2 * math.pi * h)
    L = toeInv(l)

    cs = get_Cs(L, a_, b_)
    C_0 = cs[0]
    C_mid = cs[1]
    C_max = cs[2]

    # Interpolate the three values for C so that:
    # At s=0: dC/ds = C_0, C=0
    # At s=0.8: C=C_mid
    # At s=1.0: C=C_max

    mid = 0.8
    mid_inv = 1.25

    C = None
    t = None
    k_0 = None
    k_1 = None
    k_2 = None

    if s < mid:

        t = mid_inv * s

        k_1 = mid * C_0
        k_2 = 1 - k_1 / C_mid

        C = t * k_1 / (1 - k_2 * t)

    else:

        t = (s - mid) / (1 - mid)

        k_0 = C_mid
        k_1 = (1 - mid) * C_mid * C_mid * mid_inv * mid_inv / C_0
        k_2 = 1 - (k_1) / (C_max - C_mid)

        C = k_0 + t * k_1 / (1 - k_2 * t)

    rgb = oklabToLinearSrgb((L, C * a_, C * b_))
    return linearToSrgb(rgb)


def srgbToOkhsl(rgb: tuple[float, float, float]) -> tuple[float, float, float]:
    rgb = max(rgb[0], 1e-5), max(rgb[1], 1e-5), max(rgb[2], 1e-5)
    lab = linearSrgbToOklab(srgbToLinear(rgb))

    C = math.sqrt(lab[1] * lab[1] + lab[2] * lab[2])
    a_ = lab[1] / C
    b_ = lab[2] / C

    L = lab[0]
    h = 0.5 + 0.5 * math.atan2(-lab[2], -lab[1]) / math.pi

    cs = get_Cs(L, a_, b_)
    C_0 = cs[0]
    C_mid = cs[1]
    C_max = cs[2]

    # Inverse of the interpolation in okhsl_to_srgb:

    mid = 0.8
    mid_inv = 1.25

    if C < C_mid:

        k_1 = mid * C_0
        k_2 = 1 - k_1 / C_mid

        t = C / (k_1 + k_2 * C)
        s = t * mid

    else:

        k_0 = C_mid
        k_1 = (1 - mid) * C_mid * C_mid * mid_inv * mid_inv / C_0
        k_2 = 1 - (k_1) / (C_max - C_mid)

        t = (C - k_0) / (k_1 + k_2 * (C - k_0))
        s = mid + (1 - mid) * t

    l = toe(L)
    return h * 360, s * 100, l * 100


def getOrDefault(l: list[str], default: str) -> str:
    try:
        s = l.pop()
        return s
    except:
        return default


class SettingsPerColorModel:
    def __init__(self, colorModel: ColorModel) -> None:
        settings = Krita.instance().readSetting(DOCKER_NAME, colorModel.displayName(), "")  # type: ignore
        try:
            self.initFrom(settings)
        except:
            self.initFrom("")

    def initFrom(self, settings: str):
        s = [] if len(settings) == 0 else list(reversed(settings.split(",")))
        self.enabled = getOrDefault(s, "True") == "True"
        self.barEnabled = getOrDefault(s, "True") == "True"
        self.shape = WheelShape(int(getOrDefault(s, "0")))
        self.displayChannels = getOrDefault(s, "True") == "True"
        self.swapAxes = getOrDefault(s, "False") == "True"
        self.reverseX = getOrDefault(s, "False") == "True"
        self.reverseY = getOrDefault(s, "False") == "True"
        self.rotation = float(getOrDefault(s, "0"))
        self.ringEnabled = getOrDefault(s, "False") == "True"
        self.ringThickness = float(getOrDefault(s, "0"))
        self.ringMargin = float(getOrDefault(s, "0"))
        self.ringRotation = float(getOrDefault(s, "0"))
        self.ringReversed = getOrDefault(s, "False") == "True"
        self.wheelRotateWithRing = getOrDefault(s, "False") == "True"
        self.lockedChannelIndex = int(getOrDefault(s, "0"))
        self.colorfulLockedChannel = getOrDefault(s, "False") == "True"
        self.clipToSrgbGamut = getOrDefault(s, "False") == "True"

    def write(self, colorModel: ColorModel):
        s = [
            self.enabled,
            self.barEnabled,
            int(self.shape),
            self.displayChannels,
            self.swapAxes,
            self.reverseX,
            self.reverseY,
            self.rotation,
            self.ringEnabled,
            self.ringThickness,
            self.ringMargin,
            self.ringRotation,
            self.ringReversed,
            self.wheelRotateWithRing,
            self.lockedChannelIndex,
            self.colorfulLockedChannel,
            self.clipToSrgbGamut,
        ]
        Krita.instance().writeSetting(DOCKER_NAME, colorModel.displayName(), ",".join([str(x) for x in s]))  # type: ignore


class GlobalSettings:
    def __init__(self):
        settings: str = Krita.instance().readSetting(DOCKER_NAME, "global", "")  # type: ignore
        try:
            self.initFrom(settings)
        except:
            self.initFrom("")

        order: str = Krita.instance().readSetting(DOCKER_NAME, "displayOrder", "")  # type: ignore
        orderList = order.split(",")
        self.displayOrder = (
            list(range(len(ColorModel)))
            if len(orderList) != len(ColorModel)
            else [int(cmi) for cmi in orderList]
        )

    def initFrom(self, settings: str):
        s = [] if len(settings) == 0 else list(reversed(settings.split(",")))
        self.outOfGamutColorEnabled = getOrDefault(s, "True") == "True"
        self.outOfGamutColor = (
            float(getOrDefault(s, "0.5")),
            float(getOrDefault(s, "0.5")),
            float(getOrDefault(s, "0.5")),
        )
        self.barHeight = int(getOrDefault(s, "20"))
        self.dontSyncIfOutOfGamut = getOrDefault(s, "True") == "True"
        self.pWidth = int(getOrDefault(s, "400"))
        self.pBarHeight = int(getOrDefault(s, "20"))
        self.pShortcut = getOrDefault(s, "Y")
        self.pEnableColorModelSwitcher = getOrDefault(s, "False") == "True"
        self.currentColorModel = ColorModel(int(getOrDefault(s, "0")))

    def write(self):
        s = [
            self.outOfGamutColorEnabled,
            self.outOfGamutColor[0],
            self.outOfGamutColor[1],
            self.outOfGamutColor[2],
            self.barHeight,
            self.dontSyncIfOutOfGamut,
            self.pWidth,
            self.pBarHeight,
            self.pShortcut,
            self.pEnableColorModelSwitcher,
            int(self.currentColorModel),
        ]
        Krita.instance().writeSetting(  # type: ignore
            DOCKER_NAME, "global", ",".join([str(x) for x in s])
        )
        Krita.instance().writeSetting(  # type: ignore
            DOCKER_NAME, "displayOrder", ",".join([str(x) for x in self.displayOrder])
        )
