#version 410 core

vec3 colorToRgb(vec3 color) {
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
            return vec3(1.0);
    }
}
