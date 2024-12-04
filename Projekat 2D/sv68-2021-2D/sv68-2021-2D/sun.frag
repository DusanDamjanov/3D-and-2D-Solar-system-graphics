#version 330 core

in vec2 TexCoord;       // Primanje teksturnih koordinata iz vertex šejdera
out vec4 FragColor;     // Izlazna boja fragmenta

uniform sampler2D sunTexture; // Tekstura za Sunce

void main() {
    FragColor = texture(sunTexture, TexCoord); // Dohvatanje boje iz teksture
}
