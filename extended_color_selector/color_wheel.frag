#version 410 core

uniform float constant;
uniform vec2 variables;
uniform int constantPos;
uniform vec3 lim_min;
uniform vec3 lim_max;
uniform vec3 outOfGamut;
uniform int axesConfig;
uniform int res;
uniform float rotation;
uniform float ringThickness;
uniform float ringMargin;
uniform float ringRotation;
out vec4 out_color;

vec3 colorToSrgb(vec3 color);

vec3 getColorCoordAndAntialias(vec2 p, float normalizedRingThickness);

vec4 drawWheel(vec2 p) {
    float s = sin(rotation);
    float c = cos(rotation);
    p = vec2(p.x * c - p.y * s, p.x * s + p.y * c);

    if(((axesConfig >> 1) & 1) == 1) {
        p.x = -p.x;
    }
    if(((axesConfig >> 2) & 1) == 1) {
        p.y = -p.y;
    }

    vec3 colorCoordAndAntialias = getColorCoordAndAntialias(p, (ringThickness + ringMargin) / (res / 2));
    vec2 colorCoord = colorCoordAndAntialias.xy;
    if(((axesConfig >> 0) & 1) == 1) {
        colorCoord = colorCoord.yx;
    }
    float antialias = colorCoordAndAntialias.z;
    if(any(lessThan(colorCoord, vec2(0.0)))) {
        return vec4(0.0);
    }

    vec3 t;
    switch(constantPos) {
        case 0:
            t = vec3(constant, colorCoord.x, colorCoord.y);
            break;
        case 1:
            t = vec3(colorCoord.x, constant, colorCoord.y);
            break;
        case 2:
            t = vec3(colorCoord.x, colorCoord.y, constant);
            break;
    }

    vec3 color = mix(lim_min, lim_max, t);
    color = colorToSrgb(color);
    if(any(greaterThan(color, vec3(1.0))) || any(lessThan(color, vec3(0.0)))) {
        color = all(greaterThan(outOfGamut, vec3(0.0))) ? outOfGamut : clamp(color, vec3(0.0), vec3(1.0));
    }

    return vec4(color, antialias);
}

vec4 drawRing(float x, float dist) {
    vec3 t;
    switch(constantPos) {
        case 0:
            t = vec3(x, variables.x, variables.y);
            break;
        case 1:
            t = vec3(variables.x, x, variables.y);
            break;
        case 2:
            t = vec3(variables.x, variables.y, x);
            break;
    }

    vec3 color = mix(lim_min, lim_max, t);
    color = colorToSrgb(color);
    if(any(greaterThan(color, vec3(1.0))) || any(lessThan(color, vec3(0.0)))) {
        color = all(greaterThan(outOfGamut, vec3(0.0))) ? outOfGamut : clamp(color, vec3(0.0), vec3(1.0));
    }

    const float SMOOTH = 1.5;
    float antialiasing = abs(smoothstep(res * 0.5 - ringThickness - SMOOTH, res * 0.5 - ringThickness + SMOOTH, dist) - smoothstep(res * 0.5 - SMOOTH, res * 0.5 + SMOOTH, dist));
    return vec4(color, antialiasing);
}

void main() {
    vec2 coord = gl_FragCoord.xy;
    vec2 uv = coord / res;
    vec2 p = uv * 2.0 - 1.0;
    float d = distance(coord, vec2(res) * 0.5);

    vec3 color = vec3(0.0);

    vec4 wheel = drawWheel(p);
    color = mix(color, wheel.rgb, wheel.a);

    // --BEGIN RING RENDERING--
    if(ringThickness > 0 && d < res * 0.5 && d > res * 0.5 - ringThickness) {
        float ringValue = fract((atan(p.y, p.x) + ringRotation) / 2.0 / 3.141592653589 + 0.5);
        if(((axesConfig >> 3) & 1) == 1) {
            ringValue = 1.0 - ringValue;
        }

        vec4 ring = drawRing(ringValue, d);
        color = mix(color, ring.rgb, ring.a);
    }
    // --END RING RENDERING--

    out_color = vec4(color, 1.0);
}
