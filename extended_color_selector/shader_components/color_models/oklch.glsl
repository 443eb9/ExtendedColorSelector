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

vec3 oklabToSrgb(vec3 color) {
    float lightness = color.x;
    float a = color.y;
    float b = color.z;

    float l_ = lightness + 0.3963377774 * a + 0.2158037573 * b;
    float m_ = lightness - 0.1055613458 * a - 0.0638541728 * b;
    float s_ = lightness - 0.0894841775 * a - 1.2914855480 * b;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    float red = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    float green = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    float blue = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;

    return linearToSrgb(vec3(red, green, blue));
}

vec3 colorToSrgb(vec3 color) {
    float l = color.x;
    float hue = radians(color.z);
    float a = color.y * cos(hue);
    float b = color.y * sin(hue);
    return oklabToSrgb(vec3(l, a, b));
}
