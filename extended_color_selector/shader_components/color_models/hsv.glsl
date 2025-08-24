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

vec3 colorToSrgb(vec3 color) {
    float v = color.z;
    float w = (1.0 - color.y) * v;

    float h = color.x / 60.0;
    int i = int(floor(h));
    float f = h - float(i);
    if(i % 2 == 1) {
        f = 1.0 - f;
    }

    float n = w + f * (v - w);

    switch(i) {
        case 0:
            return vec3(v, n, w);
        case 1:
            return vec3(n, v, w);
        case 2:
            return vec3(w, v, n);
        case 3:
            return vec3(w, n, v);
        case 4:
            return vec3(n, w, v);
        case 5:
            return vec3(v, w, n);
        default:
            return vec3(v, n, w);
    }
}
