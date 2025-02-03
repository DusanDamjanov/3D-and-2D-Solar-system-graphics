//#version 330 core
//
//in vec2 TexCoord; // Receiving from vertex shader
//out vec4 FragColor;
//uniform vec3 objectColor;
//
//void main() {
//    FragColor = vec4(objectColor, 1.0); // Using uniform color
//}
//

#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D sphereTexture;  // Add texture uniform

void main() {
    FragColor = texture(sphereTexture, TexCoord); // Use texture color
}
