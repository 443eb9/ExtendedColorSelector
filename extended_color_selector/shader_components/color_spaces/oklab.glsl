#version 410 core

float invGamma(float x) {
    if(x <= 0.0) {
        return x;
    }
    if(x <= 0.04045) {
        return x / 12.92;
    }
    return pow((x + 0.055) / 1.055, 2.4);
}

vec3 colorToRgb(vec3 color) {
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

    return vec3(invGamma(red), invGamma(green), invGamma(blue));
}
