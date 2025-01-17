#version 330 core

out vec4 FragColor;

in vec2 TexCoord; // Teksturne koordinate iz vertex šejdera

uniform sampler2D moonTexture; // Tekstura planete

void main()
{
    FragColor = texture(moonTexture, TexCoord); // Prikaz teksture
}
