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
out vec4 out_color;

vec3 colorToSrgb(vec3 color);

vec2 getColorCoord(vec2 p, float normalizedRingThickness);

void drawWheel(vec2 p) {
    float s = sin(rotation);
    float c = cos(rotation);
    p = vec2(p.x * c - p.y * s, p.x * s + p.y * c);

    if(((axesConfig >> 0) & 1) == 1) {
        p = p.yx;
    }
    if(((axesConfig >> 1) & 1) == 1) {
        p.x = -p.x;
    }
    if(((axesConfig >> 2) & 1) == 1) {
        p.y = -p.y;
    }

    vec2 colorCoord = getColorCoord(p, ringThickness / (res / 2));
    if(any(lessThan(colorCoord, vec2(0.0)))) {
        out_color = vec4(0.0);
        return;
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

    out_color = vec4(color, 1.0);
}

void drawRing(float x) {
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

    out_color = vec4(color, 1.0);
}

void main(void) {
    vec2 coord = gl_FragCoord.xy;
    vec2 uv = coord / res;
    vec2 p = uv * 2.0 - 1.0;
    float d = distance(coord, vec2(res) * 0.5);

    drawWheel(p);

    if(ringThickness > 0 && d < res * 0.5 && d > res * 0.5 - ringThickness) {
        drawRing(atan(p.y, p.x) / 2.0 / 3.141592653589 + 0.5);
    }
}
