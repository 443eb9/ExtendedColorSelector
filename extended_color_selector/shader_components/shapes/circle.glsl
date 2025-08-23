#version 410 core

vec2 getColorCoord(vec2 p) {
    float r = length(p);
    if(r > 1.0) {
        return vec2(-1.0);
    }
    return vec2(r, atan(p.y, p.x) / 2.0 / 3.141592653589 + 0.5);
}
