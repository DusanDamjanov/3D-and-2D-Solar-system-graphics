#version 330 core

// Ulaz iz vertex shader-a
in vec2 TexCoord;

// Izlazna boja fragmenta
out vec4 FragColor;

// Uniform promenljive
uniform sampler2D sunTexture;

void main() {
    // Koristi samo teksturu za prikaz Sunca
    FragColor = texture(sunTexture, TexCoord);
}
