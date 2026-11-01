#version 330 core

layout(location = 0) in vec3 position; // vstup: poloha vrcholu

uniform mat4 mvp; // transformace, stejná pro všechny vrcholy

void main() {
    gl_Position = mvp * vec4(position, 1.0); // výstup: poloha na obrazovce
}