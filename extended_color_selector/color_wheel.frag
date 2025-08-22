#version 410 core

uniform float constant;
uniform int constantPos;
uniform int res;
out vec4 out_color;

vec3 getColor(vec2 colorCoord, float constant, int constantPos);

vec3 colorToRgb(vec3 color);

vec2 getColorCoord(vec2 uv);

void main(void) {
    vec2 uv = gl_FragCoord.xy / res;
    vec2 colorCoord = getColorCoord(uv);
    vec3 color = getColor(colorCoord, constant, constantPos);
    color = colorToRgb(color);
    out_color = vec4(color, 1.0);
}
