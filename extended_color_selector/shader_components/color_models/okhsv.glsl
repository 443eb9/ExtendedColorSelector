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

// The following color model conversion code is translated from Björn Ottosson's code under MIT license
// 
// Original license:
// 
// Copyright (c) 2021 Björn Ottosson

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

struct Lab {
    float L;
    float a;
    float b;
};
struct RGB {
    float r;
    float g;
    float b;
};

struct HSV {
    float h;
    float s;
    float v;
};
struct LC {
    float L;
    float C;
};

// Alternative representation of (L_cusp, C_cusp)
// Encoded so S = C_cusp/L_cusp and T = C_cusp/(1-L_cusp) 
// The maximum value for C in the triangle is then found as fmin(S*L, T*(1-L)), for a given L
struct ST {
    float S;
    float T;
};

// inverse toe function for L_r
float toe_inv(float x) {
    const float k_1 = 0.206;
    const float k_2 = 0.03;
    const float k_3 = (1. + k_1) / (1. + k_2);
    return (x * x + k_1 * x) / (k_3 * (x + k_2));
}

ST to_ST(LC cusp) {
    float L = cusp.L;
    float C = cusp.C;
    return ST(C / L, C / (1 - L));
}

RGB oklab_to_linear_srgb(Lab c) {
    float l_ = c.L + 0.3963377774 * c.a + 0.2158037573 * c.b;
    float m_ = c.L - 0.1055613458 * c.a - 0.0638541728 * c.b;
    float s_ = c.L - 0.0894841775 * c.a - 1.2914855480 * c.b;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    float r = +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    float g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    float b = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;

    return RGB(r, g, b);
}

// Finds the maximum saturation possible for a given hue that fits in sRGB
// Saturation here is defined as S = C/L
// a and b must be normalized so a^2 + b^2 == 1
float compute_max_saturation(float a, float b) {
    // Max saturation will be when one of r, g or b goes below zero.

    // Select different coefficients depending on which component goes below zero first
    float k0, k1, k2, k3, k4, wl, wm, ws;

    if(-1.88170328 * a - 0.80936493 * b > 1) {
        // Red component
        k0 = +1.19086277;
        k1 = +1.76576728;
        k2 = +0.59662641;
        k3 = +0.75515197;
        k4 = +0.56771245;
        wl = +4.0767416621;
        wm = -3.3077115913;
        ws = +0.2309699292;
    } else if(1.81444104 * a - 1.19445276 * b > 1) {
        // Green component
        k0 = +0.73956515;
        k1 = -0.45954404;
        k2 = +0.08285427;
        k3 = +0.12541070;
        k4 = +0.14503204;
        wl = -1.2684380046;
        wm = +2.6097574011;
        ws = -0.3413193965;
    } else {
        // Blue component
        k0 = +1.35733652;
        k1 = -0.00915799;
        k2 = -1.15130210;
        k3 = -0.50559606;
        k4 = +0.00692167;
        wl = -0.0041960863;
        wm = -0.7034186147;
        ws = +1.7076147010;
    }

    // Approximate max saturation using a polynomial:
    float S = k0 + k1 * a + k2 * b + k3 * a * a + k4 * a * b;

    // Do one step Halley's method to get closer
    // this gives an error less than 10e6, except for some blue hues where the dS/dh is close to infinite
    // this should be sufficient for most applications, otherwise do two/three steps 

    float k_l = +0.3963377774 * a + 0.2158037573 * b;
    float k_m = -0.1055613458 * a - 0.0638541728 * b;
    float k_s = -0.0894841775 * a - 1.2914855480 * b;

    {
        float l_ = 1. + S * k_l;
        float m_ = 1. + S * k_m;
        float s_ = 1. + S * k_s;

        float l = l_ * l_ * l_;
        float m = m_ * m_ * m_;
        float s = s_ * s_ * s_;

        float l_dS = 3. * k_l * l_ * l_;
        float m_dS = 3. * k_m * m_ * m_;
        float s_dS = 3. * k_s * s_ * s_;

        float l_dS2 = 6. * k_l * k_l * l_;
        float m_dS2 = 6. * k_m * k_m * m_;
        float s_dS2 = 6. * k_s * k_s * s_;

        float f = wl * l + wm * m + ws * s;
        float f1 = wl * l_dS + wm * m_dS + ws * s_dS;
        float f2 = wl * l_dS2 + wm * m_dS2 + ws * s_dS2;

        S = S - f * f1 / (f1 * f1 - 0.5 * f * f2);
    }

    return S;
}

// finds L_cusp and C_cusp for a given hue
// a and b must be normalized so a^2 + b^2 == 1
LC find_cusp(float a, float b) {
	// First, find the maximum saturation (saturation S = C/L)
    float S_cusp = compute_max_saturation(a, b);

	// Convert to linear sRGB to find the first point where at least one of r,g or b >= 1:
    RGB rgb_at_max = oklab_to_linear_srgb(Lab(1, S_cusp * a, S_cusp * b));
    float L_cusp = pow(1. / max(max(rgb_at_max.r, rgb_at_max.g), rgb_at_max.b), 1.0 / 3.0);
    float C_cusp = L_cusp * S_cusp;

    return LC(L_cusp, C_cusp);
}

RGB okhsv_to_srgb(HSV hsv) {
    float h = hsv.h;
    float s = hsv.s;
    float v = hsv.v;

    const float pi = 3.141592653589;
    float a_ = cos(2. * pi * h);
    float b_ = sin(2. * pi * h);

    LC cusp = find_cusp(a_, b_);
    ST ST_max = to_ST(cusp);
    float S_max = ST_max.S;
    float T_max = ST_max.T;
    float S_0 = 0.5;
    float k = 1 - S_0 / S_max;

	// first we compute L and V as if the gamut is a perfect triangle:

	// L, C when v==1:
    float L_v = 1 - s * S_0 / (S_0 + T_max - T_max * k * s);
    float C_v = s * T_max * S_0 / (S_0 + T_max - T_max * k * s);

    float L = v * L_v;
    float C = v * C_v;

	// then we compensate for both toe and the curved top part of the triangle:
    float L_vt = toe_inv(L_v);
    float C_vt = C_v * L_vt / L_v;

    float L_new = toe_inv(L);
    C = C * L_new / L;
    L = L_new;

    RGB rgb_scale = oklab_to_linear_srgb(Lab(L_vt, a_ * C_vt, b_ * C_vt));
    float scale_L = pow(1. / max(max(rgb_scale.r, rgb_scale.g), max(rgb_scale.b, 0.)), 1.0 / 3.0);

    L = L * scale_L;
    C = C * scale_L;

    RGB rgb = oklab_to_linear_srgb(Lab(L, C * a_, C * b_));
    return RGB(invGamma(rgb.r), invGamma(rgb.g), invGamma(rgb.b));
}

vec3 colorToSrgb(vec3 color) {
    RGB rgb = okhsv_to_srgb(HSV(color.x / 360, color.y / 100, color.z / 100));
    return vec3(rgb.r, rgb.g, rgb.b);
}
