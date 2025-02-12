#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D asteroidTexture;
uniform float glowIntensity; // Faktor svetljenja

void main() {
    // Učitavanje boje iz teksture
    vec4 texColor = texture(asteroidTexture, TexCoord);
    
    // Primena plave nijanse na asteroid
    vec3 baseColor = mix(texColor.rgb, vec3(0.3, 0.5, 1.2), 0.8); 

    // Dodavanje svetlećeg efekta
    vec3 emissiveColor = baseColor + vec3(glowIntensity);

    // Osiguravanje da vrednosti ostanu u granicama [0,1]
    FragColor = vec4(clamp(emissiveColor, 0.0, 1.0), texColor.a);
}
