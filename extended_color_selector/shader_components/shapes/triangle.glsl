#version 410 core

// uniform float rotation;

vec2 getColorCoord(vec2 uv) {
    vec2 p = uv * 2.0 - 1.0;

    const float PI = 3.1415926535897932384626433832795;
    const float RAD_120 = PI * 120.0 / 180.0;
    const vec2 V0 = vec2(cos(RAD_120 * 0.0), sin(RAD_120 * 0.0));
    const vec2 V1 = vec2(cos(RAD_120 * 1.0), sin(RAD_120 * 1.0));
    const vec2 V2 = vec2(cos(RAD_120 * 2.0), sin(RAD_120 * 2.0));
    const vec2 VC = (V1 + V2) / 2.0;
    const vec2 VH = VC - V0;
    const float A = distance(V0, V1);
    const float H = length(VH);

    float y = dot(p - V0, VH / H) / H;
    vec2 b = p - mix(V0, V1, y);
    if(dot(b, V2 - V1) < 0.0) {
        return vec2(-1.0);
    }
    float x = length(b) / (y * A);

    if(x < 0.0 || y < 0.0 || x > 1.0 || y > 1.0) {
        return vec2(-1.0);
    }
    return vec2(x, y);
}
