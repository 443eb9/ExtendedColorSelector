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

struct HSL {
    float h;
    float s;
    float l;
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

// Returns a smooth approximation of the location of the cusp
// This polynomial was created by an optimization process
// It has been designed so that S_mid < S_max and T_mid < T_max
ST get_ST_mid(float a_, float b_) {
    float S = 0.11516993 + 1. / (+7.44778970 + 4.15901240 * b_ + a_ * (-2.19557347 + 1.75198401 * b_ + a_ * (-2.13704948 - 10.02301043 * b_ + a_ * (-4.24894561 + 5.38770819 * b_ + 4.69891013 * a_))));

    float T = 0.11239642 + 1. / (+1.61320320 - 0.68124379 * b_ + a_ * (+0.40370612 + 0.90148123 * b_ + a_ * (-0.27087943 + 0.61223990 * b_ + a_ * (+0.00299215 - 0.45399568 * b_ - 0.14661872 * a_))));

    return ST(S, T);
}

// Finds intersection of the line defined by 
// L = L0 * (1 - t) + t * L1;
// C = t * C1;
// a and b must be normalized so a^2 + b^2 == 1
float find_gamut_intersection(float a, float b, float L1, float C1, float L0, LC cusp) {
	// Find the intersection for upper and lower half seprately
    float t;
    if(((L1 - L0) * cusp.C - (cusp.L - L0) * C1) <= 0.) {
		// Lower half

        t = cusp.C * L0 / (C1 * cusp.L + cusp.C * (L0 - L1));
    } else {
		// Upper half

		// First intersect with triangle
        t = cusp.C * (L0 - 1.) / (C1 * (cusp.L - 1.) + cusp.C * (L0 - L1));

		// Then one step Halley's method
        {
            float dL = L1 - L0;
            float dC = C1;

            float k_l = +0.3963377774 * a + 0.2158037573 * b;
            float k_m = -0.1055613458 * a - 0.0638541728 * b;
            float k_s = -0.0894841775 * a - 1.2914855480 * b;

            float l_dt = dL + dC * k_l;
            float m_dt = dL + dC * k_m;
            float s_dt = dL + dC * k_s;

			// If higher accuracy is required, 2 or 3 iterations of the following block can be used:
            {
                float L = L0 * (1. - t) + t * L1;
                float C = t * C1;

                float l_ = L + C * k_l;
                float m_ = L + C * k_m;
                float s_ = L + C * k_s;

                float l = l_ * l_ * l_;
                float m = m_ * m_ * m_;
                float s = s_ * s_ * s_;

                float ldt = 3 * l_dt * l_ * l_;
                float mdt = 3 * m_dt * m_ * m_;
                float sdt = 3 * s_dt * s_ * s_;

                float ldt2 = 6 * l_dt * l_dt * l_;
                float mdt2 = 6 * m_dt * m_dt * m_;
                float sdt2 = 6 * s_dt * s_dt * s_;

                float r = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s - 1;
                float r1 = 4.0767416621 * ldt - 3.3077115913 * mdt + 0.2309699292 * sdt;
                float r2 = 4.0767416621 * ldt2 - 3.3077115913 * mdt2 + 0.2309699292 * sdt2;

                float u_r = r1 / (r1 * r1 - 0.5 * r * r2);
                float t_r = -r * u_r;

                float g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s - 1;
                float g1 = -1.2684380046 * ldt + 2.6097574011 * mdt - 0.3413193965 * sdt;
                float g2 = -1.2684380046 * ldt2 + 2.6097574011 * mdt2 - 0.3413193965 * sdt2;

                float u_g = g1 / (g1 * g1 - 0.5 * g * g2);
                float t_g = -g * u_g;

                float b = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s - 1;
                float b1 = -0.0041960863 * ldt - 0.7034186147 * mdt + 1.7076147010 * sdt;
                float b2 = -0.0041960863 * ldt2 - 0.7034186147 * mdt2 + 1.7076147010 * sdt2;

                float u_b = b1 / (b1 * b1 - 0.5 * b * b2);
                float t_b = -b * u_b;

                t_r = u_r >= 0. ? t_r : 1e9;
                t_g = u_g >= 0. ? t_g : 1e9;
                t_b = u_b >= 0. ? t_b : 1e9;

                t += min(t_r, min(t_g, t_b));
            }
        }
    }

    return t;
}

struct Cs {
    float C_0;
    float C_mid;
    float C_max;
};
Cs get_Cs(float L, float a_, float b_) {
    LC cusp = find_cusp(a_, b_);

    float C_max = find_gamut_intersection(a_, b_, L, 1, L, cusp);
    ST ST_max = to_ST(cusp);

	// Scale factor to compensate for the curved part of gamut shape:
    float k = C_max / min((L * ST_max.S), (1 - L) * ST_max.T);

    float C_mid;
    {
        ST ST_mid = get_ST_mid(a_, b_);

		// Use a soft minimum function, instead of a sharp triangle shape to get a smooth value for chroma.
        float C_a = L * ST_mid.S;
        float C_b = (1. - L) * ST_mid.T;
        C_mid = 0.9 * k * sqrt(sqrt(1. / (1. / (C_a * C_a * C_a * C_a) + 1. / (C_b * C_b * C_b * C_b))));
    }

    float C_0;
    {
		// for C_0, the shape is independent of hue, so ST are constant. Values picked to roughly be the average values of ST.
        float C_a = L * 0.4;
        float C_b = (1. - L) * 0.8;

		// Use a soft minimum function, instead of a sharp triangle shape to get a smooth value for chroma.
        C_0 = sqrt(1. / (1. / (C_a * C_a) + 1. / (C_b * C_b)));
    }

    return Cs(C_0, C_mid, C_max);
}

RGB okhsl_to_srgb(HSL hsl) {
    float h = hsl.h;
    float s = hsl.s;
    float l = hsl.l;

    if(l == 1.0) {
        return RGB(1., 1., 1.);
    } else if(l == 0.) {
        return RGB(0., 0., 0.);
    }

    const float pi = 3.141592653589;
    float a_ = cos(2. * pi * h);
    float b_ = sin(2. * pi * h);
    float L = toe_inv(l);

    Cs cs = get_Cs(L, a_, b_);
    float C_0 = cs.C_0;
    float C_mid = cs.C_mid;
    float C_max = cs.C_max;

    float mid = 0.8;
    float mid_inv = 1.25;

    float C, t, k_0, k_1, k_2;

    if(s < mid) {
        t = mid_inv * s;

        k_1 = mid * C_0;
        k_2 = (1. - k_1 / C_mid);

        C = t * k_1 / (1. - k_2 * t);
    } else {
        t = (s - mid) / (1 - mid);

        k_0 = C_mid;
        k_1 = (1. - mid) * C_mid * C_mid * mid_inv * mid_inv / C_0;
        k_2 = (1. - (k_1) / (C_max - C_mid));

        C = k_0 + t * k_1 / (1. - k_2 * t);
    }

    RGB rgb = oklab_to_linear_srgb(Lab(L, C * a_, C * b_));
    return RGB(invGamma(rgb.r), invGamma(rgb.g), invGamma(rgb.b));
}

vec3 colorToSrgb(vec3 color) {
    RGB rgb = okhsl_to_srgb(HSL(color.x / 360, color.y / 100, color.z / 100));
    return vec3(rgb.r, rgb.g, rgb.b);
}
