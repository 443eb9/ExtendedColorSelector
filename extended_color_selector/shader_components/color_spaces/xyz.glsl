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

vec3 linearToSrgb(vec3 color) {
    return vec3(invGamma(color.r), invGamma(color.g), invGamma(color.b));
}

vec3 colorToRgb(vec3 color) {
    float x = color.x;
    float y = color.y;
    float z = color.z;

    float r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
    float g = x * -0.969266 + y * 1.8760108 + z * 0.041556;
    float b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

    return linearToSrgb(vec3(r, g, b));
}
