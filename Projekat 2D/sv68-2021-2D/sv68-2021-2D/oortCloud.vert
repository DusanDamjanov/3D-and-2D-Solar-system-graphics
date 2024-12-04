#version 330 core

layout(location = 0) in vec2 aPos; // Pozicija asteroida

uniform mat4 projection; // Uniform za projekciju

void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    gl_PointSize = 2.0; // Veličina tačke
}
