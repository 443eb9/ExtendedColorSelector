#version 410 core

vec2 getColorCoord(vec2 p, float normalizedRingThickness) {
    if(normalizedRingThickness == 0) {
        return p * 0.5 + 0.5;
    }

    float d = 2.0 - normalizedRingThickness * 2;
    float a = d / sqrt(2);
    if(any(greaterThan(abs(p), vec2(a / 2.0)))) {
        return vec2(-1);
    }

    return p / (a * 0.5) * 0.5 + 0.5;
}
