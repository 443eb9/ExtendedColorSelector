#version 410 core

vec3 getColor(vec2 colorCoord, float constant, int constant_pos) {
    switch(constant_pos) {
        case 0:
            return vec3(constant, colorCoord.x, colorCoord.y);
        case 1:
            return vec3(colorCoord.x, constant, colorCoord.y);
        case 2:
            return vec3(colorCoord.x, colorCoord.y, constant);
        default:
            return vec3(1);
    }
}

vec3 colorToRgb(vec3 color) {
    return color;
}
