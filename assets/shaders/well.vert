#version 330 core

layout(location = 0) in vec3 position;
uniform mat4 mvp;
uniform vec3 grid_size;
out float shade;

void main() {
    gl_Position = mvp * vec4(position, 1.0);
    shade = 1.0;
    if (grid_size.z > 0.0) {
        shade += 0.55 * position.z / grid_size.z;
        if (position.z == 0.0) {
            // Okraj okna zůstává přesně na hraně viewportu i při zaokrouhlení matice.
            gl_Position.xy = (2.0 * position.xy / grid_size.xy) * gl_Position.w;
        }
    }
}
