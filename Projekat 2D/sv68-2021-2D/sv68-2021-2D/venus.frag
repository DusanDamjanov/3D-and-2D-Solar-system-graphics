#version 330 core

in vec2 TexCoord; // Teksturne koordinate iz vertex šejdera
out vec4 FragColor; // Boja piksela

uniform sampler2D planetTexture; // Tekstura Venerinog površinskog prikaza

void main()
{
    FragColor = texture(planetTexture, TexCoord); // Učitavanje boje iz teksture
}
