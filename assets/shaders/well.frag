#version 330 core

uniform vec3 color;
in float shade;
out vec4 fragment;

void main() {
    fragment = vec4(color * shade, 1.0);
}
