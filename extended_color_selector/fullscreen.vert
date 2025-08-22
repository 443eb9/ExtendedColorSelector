#version 410 core

void main() {
    vec2 pos;
    pos = 2.0 * vec2(gl_VertexID % 2, gl_VertexID / 2) - 1.0;
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}
