from PyQt5.QtWidgets import (
    QMessageBox,
)

from pathlib import Path
import array
import math

from .models import ColorModel
from .config import AXES_LIMITS_SEGMENTS, AXES_LIMITS_OFFSET

limitsBytes = (Path(__file__).parent / "axes_limits.bytes").read_bytes()
limits = array.array("f")
limits.frombytes(limitsBytes)

expectedLen = (
    sum([(1 if cm.isColorfulable() else 0) for cm in ColorModel])
    * (AXES_LIMITS_SEGMENTS + 1)
    * 4
    * 3
)
if len(limits) != expectedLen:
    QMessageBox.critical(
        None,
        "Extended Color Selector - Gamut Clipping",
        f"Length of numbers in gamut clipping limits file not matching to config: {len(limits)} != {expectedLen}. This can be cause by modifying the AXES_LIMITS_SEGMENTS without rebaking the limits file. To know how to rebake this file, see README.",
    )


def mapAxesToLimited(
    colorModel: ColorModel,
    primary: int,
    primaryValue: float,
    secondaryAxes: tuple[float, float],
) -> tuple[float, float]:
    (mnx, mxx), (mny, mxy) = getAxesLimitsInterpolated(colorModel, primary, primaryValue)
    x, y = secondaryAxes
    return mnx * (1 - x) + mxx * x, mny * (1 - y) + mxy * y


def unmapAxesFromLimited(
    colorModel: ColorModel,
    primary: int,
    primaryValue: float,
    mappedSecondaryAxes: tuple[float, float],
) -> tuple[float, float]:
    (mnx, mxx), (mny, mxy) = getAxesLimitsInterpolated(colorModel, primary, primaryValue)
    x, y = mappedSecondaryAxes
    return (x - mnx) / max(mxx - mnx, 1e-4), (y - mny) / max(mxy - mny, 1e-4)


def getAxesLimitsInterpolated(
    colorModel: ColorModel, primary: int, primaryValue: float
) -> tuple[tuple[float, float], tuple[float, float]]:
    a = primaryValue * AXES_LIMITS_SEGMENTS
    t = a - int(a)
    (minXA, maxXA), (minYA, maxYA) = getAxesLimits(colorModel, primary, int(a))
    (minXB, maxXB), (minYB, maxYB) = getAxesLimits(
        colorModel, primary, int(math.ceil(a))
    )
    return ((minXA * (1 - t) + minXB * t), (maxXA * (1 - t) + maxXB * t)), (
        (minYA * (1 - t) + minYB * t),
        (maxYA * (1 - t) + maxYB * t),
    )


def getAxesLimits(
    colorModel: ColorModel, primary: int, primaryValue: int
) -> tuple[tuple[float, float], tuple[float, float]]:
    if not colorModel.isNotSrgbBased():
        return ((0.0, 1.0), (0.0, 1.0))

    colorModelIndex = int(colorModel) - 3
    base = (colorModelIndex * 3 + primary) * (AXES_LIMITS_SEGMENTS + 1) + primaryValue
    base *= 4
    return (
        max(limits[base] - AXES_LIMITS_OFFSET, 0),
        min(limits[base + 1] + AXES_LIMITS_OFFSET, 1),
    ), (
        max(limits[base + 2] - AXES_LIMITS_OFFSET, 0),
        min(limits[base + 3] + AXES_LIMITS_OFFSET, 1),
    )
