#version 330 core

in vec2 TexCoord;
uniform sampler2D sunTexture;
out vec4 FragColor;

void main() {
    vec4 texColor = texture(sunTexture, TexCoord);

    // Ako alfa kanal teksture nije dovoljno visok, ignoriši piksel
    if (texColor.a < 0.1) 
        discard;

    // Ako je piksel crn (ili skoro crn), zameni ga bojom sunca
    if (length(texColor.rgb) < 0.05) { // Umesto da proveravamo samo (0,0,0), dozvolimo male varijacije
        texColor.rgb = vec3(1.0, 0.5, 0.0); // Boja sunca umesto crne
    }

    FragColor = texColor;
}
