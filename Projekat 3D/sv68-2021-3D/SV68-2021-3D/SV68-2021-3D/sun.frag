#version 330 core

// Ulaz iz vertex shader-a
in vec2 TexCoord;

// Izlazna boja fragmenta
out vec4 FragColor;

// Uniform promenljive
uniform sampler2D sunTexture;

void main() {
    // Učitavanje boje iz teksture Sunca
    vec4 texColor = texture(sunTexture, TexCoord);
    
    // Emisivno osvetljenje - Sunce zrači svetlost
    vec3 emission = texColor.rgb * 3.0; // Pojačavamo sjaj Sunca [izmedju 3 i 4 je najbolje]

    // Kombinujemo teksturu i emisiju
    FragColor = vec4(emission, texColor.a);
}
