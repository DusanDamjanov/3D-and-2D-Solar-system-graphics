#version 330 core

layout (location = 0) in vec2 aPos;      // Pozicije tačaka (2D)
layout (location = 1) in vec2 aTexCoord; // Teksturne koordinate

out vec2 TexCoord; // Prosljeđivanje teksturnih koordinata ka fragment šejderu

uniform mat4 projection; // Matrica za projekciju
uniform mat4 transform;  // Matrica za transformaciju

void main() {
    gl_Position = projection * transform * vec4(aPos, 0.0, 1.0); // Postavljanje položaja
    TexCoord = aTexCoord; // Prosljeđivanje teksturnih koordinata
}
