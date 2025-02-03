#version 330 core
layout(location = 0) in vec3 aPos;       // Vertex position
layout(location = 1) in vec2 aTexCoord;  // Texture coordinate
layout(location = 2) in vec3 aNormal;    // Normal for lighting

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;  // Position of the fragment in world space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0)); // Convert to world position
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform normal to world space
    TexCoord = aTexCoord; // Pass texture coordinates
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
