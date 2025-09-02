#version 410 core

// The following color model conversion code is translated from `bevy` under MIT license
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

vec3 colorToSrgb(vec3 color) {
    float x = color.x;
    float y = color.y;
    float z = color.z;

    float r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    float g = x * -0.969266 + y * 1.8760108 + z * 0.041556;
    float b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    return linearToSrgb(vec3(r, g, b));
}
