#version 330 core

layout (location = 0) in vec3 aPos;    // Pozicija verteksa
layout (location = 1) in vec3 aNormal; // Normalni vektor verteksa
layout (location = 2) in vec2 aTexCoord; // Teksturne koordinate

out vec2 TexCoord;  // Teksturne koordinate za fragment šejder
out vec3 Normal;    // Normalizovani normalni vektor
out vec3 FragPos;   // Pozicija fragmenta u svetlosnom prostoru

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0)); // Transformisana pozicija verteksa
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal); // Normalizacija normale
    TexCoord = aTexCoord; // Prosleđivanje teksturnih koordinata

    gl_Position = projection * view * vec4(FragPos, 1.0); // Transformacija verteksa
}
