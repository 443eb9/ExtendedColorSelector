#version 410 core

vec3 colorToRgb(vec3 hsl) {
    float value = hsl.z + hsl.y * min(hsl.z, 1 - hsl.z);
    float saturation = value == 0.0 ? 0.0 : 2.0 * (1.0 - hsl.z / value);
    vec3 color = vec3(hsl.x, saturation, value);

    float v = color.z;
    float w = (1.0 - color.y) * v;

    float h = color.x / 60.0;
    int i = int(floor(h));
    float f = h - float(i);
    if(i % 2 == 1) {
        f = 1.0 - f;
    }

    float n = w + f * (v - w);

    switch(i) {
        case 0:
            return vec3(v, n, w);
        case 1:
            return vec3(n, v, w);
        case 2:
            return vec3(w, v, n);
        case 3:
            return vec3(w, n, v);
        case 4:
            return vec3(n, w, v);
        case 5:
            return vec3(v, w, n);
        default:
            return vec3(0.0);
    }
}
