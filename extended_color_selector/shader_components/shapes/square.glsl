#version 410 core

vec3 getColorCoordAndAntialias(vec2 p, float normalizedRingThickness) {
    if(normalizedRingThickness == 0.0) {
        return vec3(p * 0.5 + 0.5, 1.0);
    }

    float d = 2.0 - normalizedRingThickness * 2.0;
    float a = d / sqrt(2.0);
    if(any(greaterThan(abs(p), vec2(a / 2.0)))) {
        return vec3(-1.0);
    }

    return vec3(p / (a * 0.5) * 0.5 + 0.5, 1.0);
}
