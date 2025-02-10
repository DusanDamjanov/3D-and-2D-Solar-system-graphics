#version 330 core
layout (location = 0) in vec4 vertex; // pozicija + UV koordinate
uniform mat4 projection; // Dodaj uniform matricu
out vec2 TexCoords;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
