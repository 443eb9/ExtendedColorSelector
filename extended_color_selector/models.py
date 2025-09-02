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

        component = open(
            Path(__file__).parent
            / "shader_components"
            / "color_models"
            / f"{name}.glsl"
        ).read()[
            18:
        ]  # To strip the version directive
        return shader.replace("vec3 colorToSrgb(vec3 color);", component)

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


NON_SRGB_GAMUT_MODELS = [
    ColorModel.Oklab,
    ColorModel.Xyz,
    ColorModel.Lab,
    ColorModel.Oklch,
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


# https://bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
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

    return l, a, b

# https://bottosson.github.io/posts/oklab/#converting-from-xyz-to-oklab
# Inverse matrices are computed from the matrix in the post
def oklabToXyz(color: tuple[float, float, float]) -> tuple[float, float, float]:
    l = color[0]
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
