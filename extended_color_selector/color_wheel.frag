#version 410 core

uniform float constant;
uniform int constantPos;
uniform vec3 lim_min;
uniform vec3 lim_max;
uniform vec3 outOfGamut;
uniform int axisConfig;
uniform int res;
out vec4 out_color;

vec3 colorToSrgb(vec3 color);

vec2 getColorCoord(vec2 uv);

void main(void) {
    vec2 uv = gl_FragCoord.xy / res;
    if(((axisConfig >> 0) & 1) == 1) {
        uv = uv.yx;
    }
    if(((axisConfig >> 1) & 1) == 1) {
        uv.x = 1.0 - uv.x;
    }
    if(((axisConfig >> 2) & 1) == 1) {
        uv.y = 1.0 - uv.y;
    }

    vec2 colorCoord = getColorCoord(uv);

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
