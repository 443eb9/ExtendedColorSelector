#version 410 core

uniform int primaryIndex;
uniform vec2 secondaryValues;
uniform vec3 lim_min;
uniform vec3 lim_max;
uniform vec3 outOfGamut;
uniform float res;
out vec4 out_color;

vec3 colorToSrgb(vec3 color);

void main(void) {
    float colorCoord = gl_FragCoord.x / res;

    vec3 t;
    switch(primaryIndex) {
        case 0:
            t = vec3(colorCoord, secondaryValues.x, secondaryValues.y);
            break;
        case 1:
            t = vec3(secondaryValues.x, colorCoord, secondaryValues.y);
            break;
        case 2:
            t = vec3(secondaryValues.x, secondaryValues.y, colorCoord);
            break;
    }

    vec3 color = mix(lim_min, lim_max, t);
    color = colorToSrgb(color);
    if(any(greaterThan(color, vec3(1.0))) || any(lessThan(color, vec3(0.0)))) {
        color = all(greaterThan(outOfGamut, vec3(0.0))) ? outOfGamut : clamp(color, vec3(0.0), vec3(1.0));
    }

    out_color = vec4(color, 1.0);
}
