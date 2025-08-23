#version 410 core

vec3 getColorCoordAndAntialias(vec2 p, float normalizedRingThickness) {
    const float SMOOTH = 2;

    float r = length(p);
    float nrt = normalizedRingThickness;
    if(r > 1.0 - nrt) {
        return vec3(-1.0);
    }

    float antialias = 1.0 - smoothstep((1.0 - nrt) * res - SMOOTH, (1.0 - nrt) * res, r * res);
    return vec3(r, atan(p.y, p.x) / 2.0 / 3.141592653589 + 0.5, antialias);
}
