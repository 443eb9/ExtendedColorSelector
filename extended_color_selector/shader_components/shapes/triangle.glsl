#version 410 core

vec2 getColorCoord(vec2 p, float normalizedRingThickness) {
    float t = 1.0 - normalizedRingThickness;

    const float PI = 3.1415926535897932384626433832795;
    const float RAD_120 = PI * 120.0 / 180.0;
    vec2 v0 = vec2(cos(RAD_120 * 0.0), sin(RAD_120 * 0.0)) * t;
    vec2 v1 = vec2(cos(RAD_120 * 1.0), sin(RAD_120 * 1.0)) * t;
    vec2 v2 = vec2(cos(RAD_120 * 2.0), sin(RAD_120 * 2.0)) * t;
    vec2 vc = (v1 + v2) / 2.0;
    vec2 vh = vc - v0;
    float a = distance(v0, v1);
    float h = length(vh);

    float y = dot(p - v0, vh / h) / h;
    vec2 b = p - mix(v0, v1, y);
    if(dot(b, v2 - v1) < 0.0) {
        return vec2(-1.0);
    }
    float x = length(b) / (y * a);

    if(x < 0.0 || y < 0.0 || x > 1.0 || y > 1.0) {
        return vec2(-1.0);
    }
    return vec2(x, y);
}
