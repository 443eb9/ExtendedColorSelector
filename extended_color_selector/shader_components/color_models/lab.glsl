#version 410 core

// The following color model conversion code is translate from `bevy` under MIT license
// 
// Original license:
// 
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

float invGamma(float x) {
    if(x <= 0.0) {
        return x;
    }
    if(x <= 0.0031308) {
        return x * 12.92;
    }
    return (1.055 * pow(x, 1.0 / 2.4)) - 0.055;
}

vec3 linearToSrgb(vec3 color) {
    return vec3(invGamma(color.r), invGamma(color.g), invGamma(color.b));
}

vec3 xyzToSrgb(vec3 color) {
    float x = color.x;
    float y = color.y;
    float z = color.z;

    float r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    float g = x * -0.969266 + y * 1.8760108 + z * 0.041556;
    float b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    return linearToSrgb(vec3(r, g, b));
}

const float LAB_CIE_EPSILON = 216.0 / 24389.0;

const float LAB_CIE_KAPPA = 24389.0 / 27.0;

const vec3 XYZ_D65_WHITE = vec3(0.95047, 1.0, 1.08883);

vec3 colorToSrgb(vec3 color) {
    float l = 100. * color.x;
    float a = 100. * color.y;
    float b = 100. * color.z;

    float fy = (l + 16.0) / 116.0;
    float fx = a / 500.0 + fy;
    float fz = fy - b / 200.0;

    float fx3 = pow(fx, 3.0);
    float xr = fx3 > LAB_CIE_EPSILON ? fx3 : (116.0 * fx - 16.0) / LAB_CIE_KAPPA;

    float yr = l > LAB_CIE_EPSILON * LAB_CIE_KAPPA ? pow((l + 16.0) / 116.0, 3.0) : l / LAB_CIE_KAPPA;

    float fz3 = pow(fz, 3.0);
    float zr = fz3 > LAB_CIE_EPSILON ? fz3 : (116.0 * fz - 16.0) / LAB_CIE_KAPPA;

    float x = xr * XYZ_D65_WHITE.x;
    float y = yr * XYZ_D65_WHITE.y;
    float z = zr * XYZ_D65_WHITE.z;

    return xyzToSrgb(vec3(x, y, z));
}
