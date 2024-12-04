#version 330 core

layout(location = 0) in vec2 aPos; // Pozicija vrhova
layout(location = 1) in vec2 aTexCoord; // Teksturne koordinate

out vec2 TexCoord; // Prosleđivanje teksturnih koordinata u fragment šejder

uniform mat4 projection;
uniform mat4 transform;

void main()
{
    gl_Position = projection * transform * vec4(aPos, 0.0, 1.0); // Transformacija pozicije
    TexCoord = aTexCoord; // Prosleđivanje teksturnih koordinata
}
