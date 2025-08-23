#version 410 core

uniform float constant;
uniform int constantPos;
uniform vec3 lim_min;
uniform vec3 lim_max;
uniform vec3 outOfGamut;
uniform int res;
out vec4 out_color;

vec3 colorToRgb(vec3 color);

vec2 getColorCoord(vec2 uv);

void main(void) {
    vec2 uv = gl_FragCoord.xy / res;
    vec2 colorCoord = getColorCoord(uv);

    vec3 color = lim_max - lim_min;
    switch(constantPos) {
        case 0:
            color *= vec3(constant, colorCoord.x, colorCoord.y);
            break;
        case 1:
            color *= vec3(colorCoord.x, constant, colorCoord.y);
            break;
        case 2:
            color *= vec3(colorCoord.x, colorCoord.y, constant);
            break;
    }

    color = color + lim_min;
    color = colorToRgb(color);
    if(any(greaterThan(color, vec3(1.0)))) {
        color = all(greaterThan(outOfGamut, vec3(0.0))) ? outOfGamut : clamp(color, vec3(0.0), vec3(1.0));
    }

    out_color = vec4(color, 1.0);
}
