#version 330 core

layout (location = 0) in vec3 aPos;    // Pozicija verteksa
layout (location = 1) in vec2 aTexCoord; // Teksturne koordinate

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord; // Prosleđivanje teksturnih koordinata u fragment šejder

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
