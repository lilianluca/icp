#version 330 core

layout(location = 0) in vec3 position; // vstup: poloha vrcholu

void main() {
    gl_Position = vec4(position, 1.0); // výstup: poloha na obrazovce
}