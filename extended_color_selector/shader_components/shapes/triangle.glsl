#version 410 core

vec3 getColorCoordAndAntialias(vec2 p, float normalizedRingThickness) {
    const float SMOOTH = 2;
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
        return vec3(-1.0);
    }
    float x = length(b) / (y * a);

    if(x < 0.0 || y < 0.0 || x > 1.0 || y > 1.0) {
        return vec3(-1.0);
    }

    // `res` will be available after shader concat as a uniform variable.
    float aa = a * res * 0.5;
    float rx0 = smoothstep(0, SMOOTH, x * aa * y);
    // TODO better antialiasing for rx1
    float rx1 = 1 - smoothstep(1.0 - SMOOTH / aa * y * 2, 1.0, x);
    float ry = 1 - smoothstep(aa - SMOOTH, aa, y * aa);
    float antialias = rx0 * rx1 * ry;

    return vec3(x, y, antialias);
}
