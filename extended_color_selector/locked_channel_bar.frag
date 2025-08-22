#version 410 core

uniform vec2 variables;
uniform int constantPos;
uniform vec3 scales;
uniform int res;
out vec4 out_color;

vec3 colorToRgb(vec3 color);

void main(void) {
    float colorCoord = gl_FragCoord.x / res;

    vec3 color = scales;
    switch(constantPos) {
        case 0:
            color *= vec3(colorCoord, variables.x, variables.y);
            break;
        case 1:
            color *= vec3(variables.x, colorCoord, variables.y);
            break;
        case 2:
            color *= vec3(variables.x, variables.y, colorCoord);
            break;
    }

    color = colorToRgb(color);
    out_color = vec4(color, 1.0);
}
