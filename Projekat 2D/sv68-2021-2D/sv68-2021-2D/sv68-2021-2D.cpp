#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>      
#include <ft2build.h>
#include FT_FREETYPE_H


GLFWwindow* initializeOpenGL(int width, int height, const char* title) {
    if (!glfwInit()) {      // Inicijalizacija GLFW-a
        std::cerr << "GLFW initialization failed!" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);    // Postavljanje verzije OpenGL-a na 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);      // Kreiranje prozora
    if (!window) {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);     // Postavljanje OpenGL konteksta

    glewExperimental = GL_TRUE;     // Inicijalizacija GLEW-a
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, width, height);        // Podešavanje OpenGL viewport-a

    return window;
}

// Funkcija za učitavanje šejdera
std::string loadShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Funkcija za kreiranje šejdera
GLuint compileShader(GLenum shaderType, const char* source) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

// Funkcija za kreiranje programa
GLuint createProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
    std::string vertexSource = loadShaderSource(vertexShaderPath);
    std::string fragmentSource = loadShaderSource(fragmentShaderPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

    GLuint program = glCreateProgram();

    GLint success;
    GLchar infoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);    // Proveri kompajliranje vertex šejdera
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Greška u vertex šejderu: " << infoLog << std::endl;
    }
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);    // Proveri kompajliranje fragment šejdera
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Greška u fragment šejderu: " << infoLog << std::endl;
    }

    glGetProgramiv(program, GL_LINK_STATUS, &success);    // Proveri linkovanje programa
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::string str(infoLog);
        if (!str.empty()){
            std::cerr << "Greška pri linkovanju programa: " << infoLog << std::endl;
        }
    }
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    return program;
}

// Funkcija za ograničenje na 60 FPS
void limitFPS(std::chrono::time_point<std::chrono::high_resolution_clock>& lastFrameTime) {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    duration<double, std::milli> frameTime = now - lastFrameTime;

    if (frameTime.count() < 16.67) { // 16.67 ms = 60 FPS
        std::this_thread::sleep_for(milliseconds(16) - frameTime);
    }
    lastFrameTime = high_resolution_clock::now();
}

//Funkcija za izracunavanje projekcije (pravougaonik koord koje vidimo na ekranu
glm::mat4 calculateProjection(int screenWidth, int screenHeight, float zoomLevel, float offsetX, float offsetY) {
    // Izracunavanje aspect ratio-a ekrana da bi sve ostalo u proporiciji a ne razvuklo se jer je ekran pravougaonik)
    float aspectRatio = static_cast<float>(screenWidth) / screenHeight;

    // Obrnuti faktor zooma (veći zoomLevel = blize)
    float zoomFactor = 1.0f / zoomLevel;

    // Kreiranje ortografske projekcije sa zoom i offset parametrima
    return glm::ortho(
        (-aspectRatio * 0.5f * zoomFactor) + offsetX,  // Leva granica
        (aspectRatio * 0.5f * zoomFactor) + offsetX,   // Desna granica
        (-0.5f * zoomFactor) + offsetY,               // Donja granica
        (0.5f * zoomFactor) + offsetY,                // Gornja granica
        -1.0f,                                        // Z-blizina
        1.0f                                          // Z-daljina
    );
}

// Funkcija za učitavanje teksture
GLuint loadTexture(const char* filePath) {

    // Provera da li fajl postoji
    std::ifstream testFile(filePath);
    if (!testFile) {
        std::cerr << "File does not exist: " << filePath << std::endl;
        return 0; // Vrati 0 jer tekstura nije uspešno učitana
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);


    // Postavke teksture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Učitavanje slike
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data) {
        // Provera broja kanala
        if (nrChannels == 1) {
            // Ako je slika binarna, konvertuj je u RGB
            unsigned char* rgbData = new unsigned char[width * height * 3];
            for (int i = 0; i < width * height; ++i) {
                rgbData[i * 3 + 0] = data[i]; // Kopiraj vrednost u R kanal
                rgbData[i * 3 + 1] = data[i]; // Kopiraj vrednost u G kanal
                rgbData[i * 3 + 2] = data[i]; // Kopiraj vrednost u B kanal
            }
            // Oslobodi originalni niz podataka
            stbi_image_free(data);

            // Alociraj novi prostor za data i prekopiraj rgbData
            data = new unsigned char[width * height * 3];
            std::memcpy(data, rgbData, width * height * 3);

            delete[] rgbData; // Oslobodi privremeni niz
            nrChannels = 3;   // Postavi broj kanala na 3
        }
        if (nrChannels == 3) {
            //std::cout << "Texture is RGB format." << std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (nrChannels == 4) {
            //std::cout << "Texture is RGBA format." << std::endl;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cerr << filePath;
            std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
            stbi_image_free(data);
            return 0; // Vrati 0 jer tekstura nije u podržanom formatu
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;

        // Prikaz greške iz STB biblioteke
        const char* error = stbi_failure_reason();
        std::cerr << "STB Image error: " << error << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}

struct Character {
    GLuint TextureID;  // ID teksture
    glm::ivec2 Size;   // Veličina karaktera
    glm::ivec2 Bearing; // Offset od osnovne linije
    GLuint Advance;    // Offset do sledećeg karaktera
};

void loadFont(const std::string& fontPath, std::map<GLchar, Character>& Characters) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "ERROR FREETYPE: Nije instanciran" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "ERROR FREETYPE: Nije ucitao font: " << fontPath << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48); // Postavi veličinu fonta na 48 piksela

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Onemogući izravnavanje bajtova

    for (GLubyte c = 0; c < 128; c++) {
        // Učitaj glif
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR FREETYPE: Nije ucitao Glyph za karakter: " << c << std::endl;
            continue;
        }

        // Kreiraj teksturu za karakter
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error
                << " za karakter: " << c << std::endl;
        }

        // Postavke teksture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Čuvaj informacije o karakteru
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}


void RenderText(GLFWwindow* window, unsigned int& shader, std::string text, float x, float y, float scale, glm::vec3 color, 
    std::map<GLchar, Character>& Characters)
{
    GLuint VAO = 0;
    GLuint VBO = 0;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight));
    glUseProgram(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

   
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO za svaki karakter
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // renderuj glyph preko pravougaionika
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update VBO memoriju
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //prebaci kursor za sledeci glyph (advance je 1/64 pixela)
        x += (ch.Advance >> 6) * scale; //bitshift za 6 da dobijes vrednost u pikselima (2^6 = 64 (podeli 1/64 piksela sa 64 da dobijes kolicinu piksela))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

class Sun2D {
public:
    float x, y;         // Pozicija Sunca (0, 0)
    float size;         // Veličina (radijus)
    float rotationSpeed; // Brzina rotacije
    float currentAngle;  // Trenutni ugao rotacije

    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint textureID;


    Sun2D(float posX, float posY, float sz, float rotSpeed, GLuint program, const char* texturePath)
        : x(posX), y(posY), size(sz), rotationSpeed(rotSpeed), shaderProgram(program), currentAngle(0.0f) {
        textureID = loadTexture(texturePath); // Učitaj teksturu
        generateCircleData();
    }

    // Ažuriranje ugla rotacije (ovde vrsi rotaciju sam oko sebe)
    void update(float deltaTime) {
        currentAngle += rotationSpeed * deltaTime;
        if (currentAngle > 360.0f) currentAngle -= 360.0f;
    }

    // Generišemo podatke o tačkama kruga
    void generateCircleData() {
        int numSegments = 100; // Broj segmenata kruga
        GLfloat* vertices = new GLfloat[(numSegments + 2) * 4]; // +2 za centar i poslednju tacku

        // Prva tačka je centar kruga
        vertices[0] = x;
        vertices[1] = y;
        vertices[2] = 0.5f; // Teksturna koordinata za centar
        vertices[3] = 0.5f;

        for (int i = 0; i <= numSegments; ++i) {
            float angle = (i * 2.0f * M_PI) / numSegments;
            vertices[4 * (i + 1)] = x + cos(angle) * size; // X koordinata
            vertices[4 * (i + 1) + 1] = y + sin(angle) * size; // Y koordinata
            vertices[4 * (i + 1) + 2] = 0.5f + 0.5f * cos(angle); // Teksturna X
            vertices[4 * (i + 1) + 3] = 0.5f + 0.5f * sin(angle); // Teksturna Y
        }

        glGenVertexArrays(1, &VAO);
        if (VAO == 0) {
            std::cerr << "Nije generisan VAO!" << std::endl;
            delete[] vertices;
            return;
        }
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        if (VBO == 0) {
            std::cerr << "Nije generisan VBO!" << std::endl;
            glBindVertexArray(0);
            delete[] vertices;
            return;
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (numSegments + 2) * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        // Pozicije (layout 0)
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Teksturne koordinate (layout 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        delete[] vertices;
    }

    // Crtanje Sunca
    void draw(float deltaTime, const glm::mat4& projection) {
        update(deltaTime);

        glUseProgram(shaderProgram);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error << std::endl;
        }

        // Aktiviraj teksturu
        glActiveTexture(GL_TEXTURE0);       //uzmi teksturu koju si kreirao
        glBindTexture(GL_TEXTURE_2D, textureID);

        GLenum error1 = glGetError();
        if (error1 != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error1 << std::endl;
        }

        //Uniform za teksturu
        GLuint textureLoc = glGetUniformLocation(shaderProgram, "sunTexture");  //dohvati sun.frag promenljivu
        glUniform1i(textureLoc, 0); // Koristi teksturnu jedinicu 0 (samo cemo 1 teksturu sad imati) a ovde ce biti koriscena bindovana tekstura od gore


        // Uniform za projekciju
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");   //uzmi projectio iz sun.vert (prazna)
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection)); // neka se sada kopira ova iz CPU u tu u sun.vert

        // Uniform za rotaciju
        glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(currentAngle), glm::vec3(0.0f, 0.0f, 1.0f));    //matrica koja rotira oko Z-ose
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform"); //dohvati promenljivu iz sejdera
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform)); //koristi

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 102); // 102 [ 100 segmenata + centar + zatvaranje ]
        GLenum error2 = glGetError();
        if (error2 != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error2 << std::endl;
        }

        glBindVertexArray(0);
    }

    glm::vec2 getScreenPosition() {
        return glm::vec2(x, y);     //vraca poziciju sunca (0, 0)
    }

    struct SunBounds {      //poenta strukture je najvise da vrati radijus povecan ili umanjen zavisnosti od zooma.
        glm::vec2 center; // Centar površine
        float radius;     // Radijus površine
    };

    SunBounds getSunBounds(float zoomLevel) {
        // Računaj trenutnu poziciju Sunca uzimajući u obzir offsete
        glm::vec2 sunScreenPos = getScreenPosition();

        // Skaliraj veličinu Sunca prema zoom nivou
        float scaledSize = size * zoomLevel;

        // Vrati centar i radijus kao strukturu
        return SunBounds{ sunScreenPos, scaledSize };
    }
};

class Planet2D {
public:
    float distance;       // Udaljenost od Sunca
    float size;           // Veličina planete
    float orbitSpeed;     // Brzina orbite (rotacija oko Sunca)
    float currentOrbit;   // Trenutni ugao orbite
    float eccentricity;   // Ekscentričnost orbite
    float semiMinorAxis;  // Polumanja osa elipse
    float selfRotationSpeed;  // Brzina rotacije planete oko svoje ose
    float currentSelfRotation;  // Trenutni ugao rotacije planete oko svoje ose

    GLuint shaderProgram; // Shader program za crtanje
    GLuint VAO, VBO;      // VAO i VBO za planetu
    GLuint orbitVAO, orbitVBO; // VAO i VBO za orbitu
    GLuint textureID;     // ID teksture za planetu

    // Konstruktor
    Planet2D(float dist, float ecc, float sz, float speed, GLuint program, const char* texturePath, float selfRotSpeed)
        : distance(dist), eccentricity(ecc), size(sz), orbitSpeed(speed), shaderProgram(program),
        selfRotationSpeed(selfRotSpeed), currentOrbit(0.0f), currentSelfRotation(0.0f) {
        // Izracunavanje polumanje ose na osnovu ekscentričnosti
        semiMinorAxis = distance * sqrt(1 - eccentricity * eccentricity);
        textureID = loadTexture(texturePath);
        generateCircleData();
        generateOrbitData();
        float scalingFactor = std::max(0.5f, std::min(1.5f / distance, 1.0f));
        size *= scalingFactor;
    }

    // Update orbitu i rotaciju
    void update(float deltaTime) {
        currentOrbit += orbitSpeed * deltaTime;
        if (currentOrbit > 360.0f) currentOrbit -= 360.0f;

        currentSelfRotation += selfRotationSpeed * deltaTime;
        if (currentSelfRotation > 360.0f) currentSelfRotation -= 360.0f;
    }

    // Generisanje podataka o krugu
    void generateCircleData() {
        int numSegments = 50; 
        GLfloat* vertices = new GLfloat[(numSegments + 2) * 4];

        // Centar planete
        vertices[0] = 0.0f;
        vertices[1] = 0.0f;
        vertices[2] = 0.5f; // Teksturna koordinata X
        vertices[3] = 0.5f; // Teksturna koordinata Y

        for (int i = 0; i <= numSegments; ++i) {
            float angle = (i * 2.0f * M_PI) / numSegments;
            vertices[4 * (i + 1)] = cos(angle) * size;   // X koordinata
            vertices[4 * (i + 1) + 1] = sin(angle) * size; // Y koordinata
            vertices[4 * (i + 1) + 2] = 0.5f + 0.5f * cos(angle); // Teksturna X    [0.5 sluzi da bi iz kvadratne slike izvukli krug]
            vertices[4 * (i + 1) + 3] = 0.5f + 0.5f * sin(angle); // Teksturna Y
        }

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (numSegments + 2) * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        // Pozicije
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Teksturne koordinate
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        delete[] vertices;
    }

    // Generisanje podataka za orbitu
    void generateOrbitData() {
        int numSegments = 100;
        GLfloat* vertices = new GLfloat[numSegments * 2];

        for (int i = 0; i < numSegments; ++i) {
            float angle = (i * 2.0f * M_PI) / numSegments;
            float x = cos(angle) * distance;
            float y = sin(angle) * semiMinorAxis;
            vertices[2 * i] = x - distance * eccentricity; // Translacija elipse
            vertices[2 * i + 1] = y;
        }

        glGenVertexArrays(1, &orbitVAO);
        glBindVertexArray(orbitVAO);

        glGenBuffers(1, &orbitVBO);
        glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
        glBufferData(GL_ARRAY_BUFFER, numSegments * 2 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        delete[] vertices;
    }

    // Crtanje planete
    void draw(float deltaTime, const glm::mat4& projection) {
        update(deltaTime);

        // Aktiviraj šejder program
        glUseProgram(shaderProgram);

        // Aktiviraj teksturu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Uniform za teksturu
        GLuint textureLoc = glGetUniformLocation(shaderProgram, "planetTexture");
        glUniform1i(textureLoc, 0);

        // Uniform za projekciju
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Transformacija za planetu (orbita + rotacija oko svoje ose) ovo ce se u sejderu mnoziti sa porjekcijom (bitno mi da uzmem orbitnu poziciju i ugao da izracunam ovo)
        float angle = currentOrbit * M_PI / 180.0f;     //trenutno na orbiti uzmi i pretvori taj ugao u radijane (pi / 180)
        glm::vec2 position(cos(angle) * distance - distance * eccentricity, sin(angle) * semiMinorAxis);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        transform = glm::rotate(transform, glm::radians(currentSelfRotation), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotacija oko z ose

        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 52);
        glBindVertexArray(0);
    }

    // Crtanje orbite
    void drawOrbit(const glm::mat4& projection, GLuint orbitShaderProgram) {
        glUseProgram(orbitShaderProgram);

        // Uniform za projekciju
        GLuint projectionLoc = glGetUniformLocation(orbitShaderProgram, "projection");
        if (projectionLoc != -1) {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        glBindVertexArray(orbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, 100); // Nacrtaj orbitu kao linijsku petlju
        glBindVertexArray(0);
    }

    // Funkcija da vrati poziciju planete da bi mesec mogao da se crta
    glm::vec2 getPosition() {       //uzmi poziciju ko sto si je uzimao u draw(), i onda vrati svetovnu poziciju planete
        float angle = currentOrbit * M_PI / 180.0f;
        float x = distance * cos(angle) - distance * eccentricity;
        float y = semiMinorAxis * sin(angle);
        return glm::vec2(x, y);
    }

    struct PlanetBounds {      //poenta strukture je najvise da vrati radijus povecan ili umanjen zavisnosti od zooma.
        glm::vec2 center; // Centar površine
        float radius;     // Radijus površine
    };

    PlanetBounds getPlanetBounds(float zoomLevel) {
        // Racunaj trenutnu poziciju
        glm::vec2 sunScreenPos = getPosition();

        // Skaliraj velicinu prema zoom nivou
        float scaledSize = size * zoomLevel;
        return PlanetBounds{ sunScreenPos, scaledSize };
    }

};

class Moon2D {
public:
    float distance;        // Udaljenost od planete
    float size;            // Veličina Meseca
    float orbitSpeed;      // Brzina orbite (rotacija oko planete)
    float currentOrbit;    // Trenutni ugao orbite
    GLuint shaderProgram;  // Shader program za crtanje
    GLuint VAO, VBO;       // VAO i VBO za Mesec
    GLuint textureID;      // ID teksture Meseca
    Planet2D& planet;      // Referenca na planetu oko koje se vrti

    Moon2D(Planet2D& parentPlanet, float dist, float sz, float speed, GLuint program, const char* texturePath)
        : planet(parentPlanet), distance(dist), size(sz), orbitSpeed(speed), shaderProgram(program), currentOrbit(0.0f) {
        textureID = loadTexture(texturePath); // Učitavanje teksture
        generateCircleData();
    }

    // Ažuriranje orbite Meseca
    void update(float deltaTime) {
        currentOrbit += orbitSpeed * deltaTime;
        if (currentOrbit > 360.0f) currentOrbit -= 360.0f;
    }

    // Generisanje podataka o krugu
    void generateCircleData() {
        int numSegments = 50; // Manje segmenata za manji objekat
        GLfloat* vertices = new GLfloat[(numSegments + 2) * 4]; // +2 za centar i poslednju tačku, *4 zbog teksturnih koordinata

        // Centar Meseca
        vertices[0] = 0.0f;
        vertices[1] = 0.0f;
        vertices[2] = 0.5f; // Teksturna koordinata X
        vertices[3] = 0.5f; // Teksturna koordinata Y

        // Generisanje tačaka na obodu kruga
        for (int i = 0; i <= numSegments; ++i) {
            float angle = (i * 2.0f * M_PI) / numSegments;
            vertices[4 * (i + 1)] = cos(angle) * size;   // X koordinata
            vertices[4 * (i + 1) + 1] = sin(angle) * size; // Y koordinata
            vertices[4 * (i + 1) + 2] = 0.5f + 0.5f * cos(angle); // Teksturna X
            vertices[4 * (i + 1) + 3] = 0.5f + 0.5f * sin(angle); // Teksturna Y
        }

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (numSegments + 2) * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        // Pozicije
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        // Teksturne koordinate
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        delete[] vertices;
    }

    // Crtanje Meseca
    void draw(float deltaTime, const glm::mat4& projection) {
        update(deltaTime);

        // Pozicija planete
        glm::vec2 planetPosition = planet.getPosition();

        // Pozicija Meseca u odnosu na planetu
        float angle = currentOrbit * M_PI / 180.0f;
        glm::vec2 moonOffset = glm::vec2(
            cos(angle) * distance,
            sin(angle) * distance
        );
        glm::vec2 moonPosition = planetPosition + moonOffset;

        // Postavljanje transformacije za Mesec
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(moonPosition, 0.0f));

        glUseProgram(shaderProgram);

        // Aktiviraj teksturu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Uniform za teksturu
        GLuint textureLoc = glGetUniformLocation(shaderProgram, "moonTexture");
        glUniform1i(textureLoc, 0);

        // Uniform za projekciju
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Uniform za transformaciju
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 52); // 50 segmenata + centar + zatvaranje
        glBindVertexArray(0);

        // Proveri OpenGL greške
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL greška kod crtanja Meseca: " << error << std::endl;
        }
    }
};

class AsteroidBelt {
public:
    std::vector<glm::vec2> asteroidPositions; // Pozicije asteroida
    GLuint VAO, VBO;                          // VAO i VBO za asteroidni pojas
    int numAsteroids;                         // Broj asteroida
    float innerRadius;                        // Unutrašnji radijus (Marsova orbita)
    float outerRadius;                        // Spoljašnji radijus (Jupiterova orbita)

    AsteroidBelt(int count, float inner, float outer)
        : numAsteroids(count), innerRadius(inner), outerRadius(outer) {
        generateAsteroids();
        setupAsteroids();
    }

    // Generisanje nasumičnih pozicija za asteroide
    void generateAsteroids() {
        for (int i = 0; i < numAsteroids; ++i) {
            float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
            float radius = innerRadius + static_cast<float>(rand()) / RAND_MAX * (outerRadius - innerRadius);
            float x = radius * cos(angle);
            float y = radius * sin(angle);
            asteroidPositions.emplace_back(x, y);
        }
    }

    // Postavljanje VAO i VBO za asteroidni pojas
    void setupAsteroids() {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, asteroidPositions.size() * sizeof(glm::vec2), asteroidPositions.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Crtanje asteroidnog pojasa
    void draw(const glm::mat4& projection, GLuint shaderProgram) {
        glUseProgram(shaderProgram);

        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        if (projectionLoc != -1) {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, numAsteroids);
        glBindVertexArray(0);
    }
};

//Funkcija za crtanje orbita
void drawOrbits(glm::mat4 projection, GLuint orbitProgram, Planet2D mercury, Planet2D venus, Planet2D earth, Planet2D mars, Planet2D jupiter,
    Planet2D saturn, Planet2D uranus, Planet2D neptune, Planet2D pluto) {
    mercury.drawOrbit(projection, orbitProgram);
    venus.drawOrbit(projection, orbitProgram);
    earth.drawOrbit(projection, orbitProgram);
    mars.drawOrbit(projection, orbitProgram);
    jupiter.drawOrbit(projection, orbitProgram);
    saturn.drawOrbit(projection, orbitProgram);
    uranus.drawOrbit(projection, orbitProgram);
    neptune.drawOrbit(projection, orbitProgram);
    pluto.drawOrbit(projection, orbitProgram);
}

//funkcija da proveri slucajne visestruke klikove
bool isOneClick(double& lastKeyPressTime) {
    double debounceDelay = 0.5;
    double currentTime = glfwGetTime(); // trenutno vreme (sekunde od pokretanja aplikacije)

    if (currentTime - lastKeyPressTime > debounceDelay) {
        lastKeyPressTime = currentTime; // update vreme poslednje obrade
        return false;
    }
    return true;
}

//Funkcija za transformaciju koordinata misa
glm::vec2 getMouseWorldPosition(GLFWwindow* window, int screenWidth, int screenHeight, glm::mat4& projection) {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Normalizovane koordinate miša (od -1 do 1)
    float normalizedX = ((float)mouseX / (float)screenWidth) * 2.0f - 1.0f;
    float normalizedY = 1.0f - ((float)mouseY / (float)screenHeight) * 2.0f;

    // Vektor u "ekran" prostoru
    glm::vec4 screenPos = glm::vec4(normalizedX, normalizedY, 0.0f, 1.0f);

    // Inverzna projekcija da dobijemo svetovni prostor
    glm::mat4 inverseProjection = glm::inverse(projection);
    glm::vec4 worldPos = inverseProjection * screenPos;

    // Vraćamo X i Y svetovne koordinate
    return glm::vec2(worldPos.x, worldPos.y);
}

bool isMouseOverPlanet(const glm::vec2& mousePos, const Planet2D::PlanetBounds objectPos, float zoomLevel) {
    float dx = mousePos.x - objectPos.center.x;
    float dy = mousePos.y - objectPos.center.y;

    //float scaledRadius = objectPos.radius * zoomLevel;


    if (dx * dx + dy * dy <= objectPos.radius * objectPos.radius) {
        return true;
    }
    else {
        return false;
    }
}

bool isMouseOverSun(const glm::vec2& mousePos, const Sun2D::SunBounds objectPos, float zoomLevel) {
    float dx = mousePos.x - objectPos.center.x;
    float dy = mousePos.y - objectPos.center.y;

    //float scaledRadius = objectPos.radius * zoomLevel;


    if (dx * dx + dy * dy <= objectPos.radius * objectPos.radius) {
        return true;
    }
    else {
        return false;
    }
}

void renderInfoBox(float x, float y, float width, float height, GLuint shaderProgram, const char* textureName) {
    GLuint texture = loadTexture(textureName);

    if (texture == 0) {
        std::cerr << "Error: Nevalidni Texture ID" << std::endl;
        return;
    }

    // Prilagodimo visinu i širinu da budu u NDC
    width = width * 2.0f; 
    height = height * 2.0f;

    // Verteksi za pravougaonik
    float vertices[6][4] = {
        {x, y, 0.0f, 0.0f},
        {x, y - height, 0.0f, 1.0f},
        {x + width, y - height, 1.0f, 1.0f},

        {x, y, 0.0f, 0.0f},
        {x + width, y - height, 1.0f, 1.0f},
        {x + width, y, 1.0f, 0.0f}
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Aktiviraj sejder
    glUseProgram(shaderProgram);

    // Binduj teksturu
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Setuj uniform za teksturu
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // Renderuj pravougaonik
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Očisti resurse
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

//Funkcija ako se desi hoves/click na planetu
void mouseHoverDetection(GLFWwindow* window, int screenWidth, int screenHeight, float zoomLevel, Sun2D& sun, Planet2D& mercury, Planet2D& earth, Planet2D & venus,
    Planet2D& mars, Planet2D& jupiter, Planet2D& saturn, Planet2D& uranus, Planet2D& neptune, Planet2D& pluto, glm::mat4 projection, GLuint textShaderProgram,
    std::map<GLchar, Character> Characters, GLuint triviaShaderProgram) {
   

    glm::vec2 mouseWorldPos = getMouseWorldPosition(window, screenWidth, screenHeight, projection);
    
    //glm::mat4 projectionText = glm::ortho(0.0f, static_cast<float>(screenWidth), 0.0f, static_cast<float>(screenHeight));
    // Postavi uniform za projekciju u šejderu za tekst
    glUseProgram(textShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //Proveri za sunce
    Sun2D::SunBounds sunBounds = sun.getSunBounds(zoomLevel);
    if (isMouseOverSun(mouseWorldPos, sunBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Sun", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "sun-trivia.png");
        }
    }
    // Proveri za planete
    Planet2D::PlanetBounds mercuryBounds = mercury.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, mercuryBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Mercury", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "mercury-trivia.png");
        }
    }
    Planet2D::PlanetBounds earthBounds = earth.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, earthBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Earth", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "earth-trivia.png");
        }
    }
    Planet2D::PlanetBounds venusBounds = venus.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, venusBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Venus", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "venus-trivia.png");
        }
    }
    Planet2D::PlanetBounds marsBounds = mars.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, marsBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Mars", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "mars-trivia.png");
        }
    }
    Planet2D::PlanetBounds jupiterBounds = jupiter.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, jupiterBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Jupiter", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "jupiter-trivia.png");
        }
    }
    Planet2D::PlanetBounds saturnBounds = saturn.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, saturnBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Saturn", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "saturn-trivia.png");
        }
    }
    Planet2D::PlanetBounds uranusBounds = uranus.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, uranusBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Uranus", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "uranus-trivia.png");
        }
    }
    Planet2D::PlanetBounds neptuneBounds = neptune.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, neptuneBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Neptune", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "neptune-trivia.png");
        }
    }
    Planet2D::PlanetBounds plutoBounds = pluto.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, plutoBounds, zoomLevel)) {
        RenderText(window, textShaderProgram, "Pluto", 0.0f, 0.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), Characters);
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, triviaShaderProgram, "pluto-trivia.png");
        }
    }
}

int main() {

    int screenWidth = 1800, screenHeight = 950;

    GLFWwindow* window = initializeOpenGL(screenWidth, screenHeight, "Suncev Sistem - 2D");
    if (!window) return -1;


    double lastClickTime = glfwGetTime();   //zapis poslednjeg klika (pomaze pri onemogucavanju slucajnih visestrukih klikova)

    float zoomLevel = 1.0f;             // pocetni nivo zooma
    float minZoom = 5.0f;               // koliko maximalno mozes da umanjis
    float maxZoom = 0.2f;               // koliko maximalno mozes da uvecas
    float offsetX = 0.0f;               // ofset za horizontalno pomeranje
    float offsetY = 0.0f;               // ofset za vertikalno pomeranje
    float panSpeed = 0.05f;             //brzina pomeranja pogleda
    bool orbitsPresent = true;          // boolean da li ce orbite biti prikazane
    float speedMultiplier = 1.0f;       // faktor ubrzanja animacije


    // Kreiranje ortografske projekcije
    glm::mat4 projection = calculateProjection(screenWidth, screenHeight, zoomLevel, offsetX, offsetY);

    std::map<GLchar, Character> Characters;     //mapa karaktera za prikazivanje teksta
    loadFont("LiberationSans-Regular.ttf", Characters); //ucitaj mapu

    //ucitavanje sejdera za tekst i dodatne informacije
    GLuint textShaderProgram = createProgram("text.vert", "text.frag");
    GLuint triviaShaderProgram = createProgram("details.vert", "details.frag");

    //ucitavanje svih sejdera za sve objekte
    GLuint sunProgram = createProgram("sun.vert", "sun.frag");
    GLuint mercuryProgram = createProgram("mercury.vert", "mercury.frag");
    GLuint earthProgram = createProgram("earth.vert", "earth.frag");
    GLuint venusProgram = createProgram("venus.vert", "venus.frag");
    GLuint marsProgram = createProgram("mars.vert", "mars.frag");
    GLuint jupiterProgram = createProgram("jupiter.vert", "jupiter.frag");
    GLuint saturnProgram = createProgram("saturn.vert", "saturn.frag");
    GLuint uranusProgram = createProgram("uranus.vert", "uranus.frag");
    GLuint neptuneProgram = createProgram("neptune.vert", "neptune.frag");
    GLuint plutoProgram = createProgram("pluto.vert", "pluto.frag");
    GLuint orbitProgram = createProgram("orbit.vert", "orbit.frag");
    GLuint moonProgram = createProgram("moon.vert", "moon.frag");
    GLuint asteroidProgram = createProgram("asteroids.vert", "asteroids.frag");
    GLuint kuiperProgram = createProgram("kuiper-oort.vert", "kuiper-oort.frag");
    GLuint oortCloudProgram = createProgram("kuiper-oort.vert", "kuiper-oort.frag");


    // Kreiranje planeta    
    Sun2D sun(0.0f, 0.0f, 0.05f, 90.0f, sunProgram, "sun-texture.jpg");                         // Sunce
    Planet2D mercury(0.1f, 0.206f, 0.02f, 150.0f, mercuryProgram, "mercury-texture.jpg", 30.0f);// Merkur
    Planet2D venus(0.15f, 0.007f, 0.03f, 120.0f, venusProgram, "venus-texture.jpeg", 30.0f);    // Venera
    Planet2D earth(0.2f, 0.017f, 0.035f, 100.0f, earthProgram, "earth-texture.jpg", 30.0f);     // Zemlja
    Planet2D mars(0.3f, 0.093f, 0.03f, 80.0f, marsProgram, "mars-texture.jpg", 30.0f);          // Mars
    Planet2D jupiter(0.5f, 0.049f, 0.06f, 40.0f, jupiterProgram, "jupiter-texture.jpg", 30.0f); // Jupiter
    Planet2D saturn(0.7f, 0.056f, 0.05f, 30.0f, saturnProgram, "saturn-texture.jpg", 30.0f);    // Saturn
    Planet2D uranus(1.0f, 0.046f, 0.04f, 20.0f, uranusProgram, "uranus-texture.jpg", 30.0f);    // Uran
    Planet2D neptune(1.3f, 0.010f, 0.04f, 10.0f, neptuneProgram, "neptune-texture.jpg", 30.0f); // Neptun
    Planet2D pluto(1.6f, 0.248f, 0.02f, 5.0f, plutoProgram, "pluto-texture.jpg", 30.0f);        // Pluton

    Moon2D moon(earth, 0.03f, 0.01f, 300.0f, moonProgram, "moon-texture.jpg");                  // Mesec oko Zemlje
    Moon2D phobos(mars, 0.06f, 0.008f, 300.0f, moonProgram, "phobos-texture.jpg");              // Fobos - Mars
    Moon2D deimos(mars, 0.15f, 0.01f, 150.0f, moonProgram, "deimos-texture.jpg");               // Deimos - Mars
    Moon2D io(jupiter, 0.1f, 0.015f, 250.0f, moonProgram, "io-texture.jpg");                    // Io - Jupiter
    Moon2D europa(jupiter, 0.15f, 0.012f, 200.0f, moonProgram, "europa-texture.jpg");           // Evropa - Jupiter
    Moon2D ganymede(jupiter, 0.25f, 0.02f, 150.0f, moonProgram, "ganymede-texture.jpg");        // Ganimed - Jupiter
    Moon2D callisto(jupiter, 0.35f, 0.018f, 100.0f, moonProgram, "callisto-texture.jpg");       // Kalisto - Jupiter
    Moon2D titan(saturn, 0.15f, 0.03f, 200.0f, moonProgram, "titan-texture.jpg");               // Titan - Saturn
    Moon2D rhea(saturn, 0.25f, 0.02f, 150.0f, moonProgram, "rhea-texture.jpg");                 // Rea - Saturn
    Moon2D iapetus(saturn, 0.4f, 0.015f, 100.0f, moonProgram, "iapetus-texture.jpg");           // Japet - Saturn
    Moon2D miranda(uranus, 0.1f, 0.02f, 180.0f, moonProgram, "miranda-texture.jpg");            // Miranda - Uran
    Moon2D ariel(uranus, 0.2f, 0.025f, 140.0f, moonProgram, "ariel-texture.jpg");               // Ariel -  Uran
    Moon2D umbriel(uranus, 0.3f, 0.02f, 100.0f, moonProgram, "umbriel-texture.jpg");            // Umbriel - Uran
    Moon2D triton(neptune, 0.15f, 0.03f, 120.0f, moonProgram, "triton-texture.jpg");            // Triton - Neptun
    
    AsteroidBelt asteroidBelt(1000, 0.35f, 0.45f);  // 1000 asteroida između Marsa i Jupitera
    AsteroidBelt kuiperBelt(1500, 1.5f, 2.0f);      // 1500 objekata između Neptuna i Plutona
    AsteroidBelt oortCloud(5000, 2.5f, 5.0f);       // 5000 objekata u Oortovom oblaku




    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        glm::mat4 projection = calculateProjection(screenWidth, screenHeight, zoomLevel, offsetX, offsetY);


        //ZUMIRANJE
        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            zoomLevel += 0.01f; // Zumiraj
            if (zoomLevel < maxZoom) zoomLevel = maxZoom; // Minimalni zoom
        }

        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            zoomLevel -= 0.01f; // Odzumiraj
            if (zoomLevel > minZoom) zoomLevel = minZoom; // Maksimalni zoom
        }

        //GASENJE
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            std::cout << "Pritisnuli ste ESCAPE i sve gasim...";
            exit(0);
        }

        //POMERANJE PO EKRANU
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            offsetY += panSpeed * zoomLevel; // Pomeranje nagore
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            offsetY -= panSpeed * zoomLevel; // Pomeranje nadole
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            offsetX -= panSpeed * zoomLevel; // Pomeranje ulevo

        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            offsetX += panSpeed * zoomLevel; // Pomeranje udesno

        }

        //ORBITE ON/OFF
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
            if (!isOneClick(lastClickTime)) {
                orbitsPresent = !orbitsPresent;
            }
        }

        //PAUZIRAJ ANIMACIJU
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            speedMultiplier = 0.0f;
        }

        //UBRZAJ/USPORI ANIMACIJU
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            if(!isOneClick(lastClickTime))
            {
                if (speedMultiplier == 0.0f) { //pauziranu animaciju mozes da pokrenes na bilo koji od ova dva tastera za brzinu
                    speedMultiplier = 1.0f;
                }
                else {
                    speedMultiplier += 50;
                    if (speedMultiplier > 200) {
                        speedMultiplier = 200.0f;
                    }
                }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            if (!isOneClick(lastClickTime)) {
                if (speedMultiplier == 0.0) {
                    speedMultiplier = 1.0;
                }
                else {
                    speedMultiplier -= 50;
                    if (speedMultiplier < 1) {
                        speedMultiplier = 1.0f;
                    }
                }
            }
        }


        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        limitFPS(lastFrameTime); // Ograničavanje na 60 FPS

        // Brisanje ekrana
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Crna pozadina

        // Crtanje Planeta
        sun.draw(deltaTime.count() * speedMultiplier, projection);
        mercury.draw(deltaTime.count() * speedMultiplier, projection);
        venus.draw(deltaTime.count() * speedMultiplier, projection);

        earth.draw(deltaTime.count() * speedMultiplier, projection);
        moon.draw(deltaTime.count() * speedMultiplier, projection);
        
        mars.draw(deltaTime.count() * speedMultiplier, projection);
        phobos.draw(deltaTime.count() * speedMultiplier, projection);
        deimos.draw(deltaTime.count() * speedMultiplier, projection);

        jupiter.draw(deltaTime.count() * speedMultiplier, projection);
        io.draw(deltaTime.count() * speedMultiplier, projection);
        europa.draw(deltaTime.count() * speedMultiplier, projection);
        ganymede.draw(deltaTime.count() * speedMultiplier, projection);
        callisto.draw(deltaTime.count() * speedMultiplier, projection);

        saturn.draw(deltaTime.count() * speedMultiplier, projection);
        titan.draw(deltaTime.count() * speedMultiplier, projection);
        rhea.draw(deltaTime.count() * speedMultiplier, projection);
        iapetus.draw(deltaTime.count() * speedMultiplier, projection);

        uranus.draw(deltaTime.count() * speedMultiplier, projection);
        miranda.draw(deltaTime.count() * speedMultiplier, projection);
        ariel.draw(deltaTime.count() * speedMultiplier, projection);
        umbriel.draw(deltaTime.count() * speedMultiplier, projection);

        neptune.draw(deltaTime.count() * speedMultiplier, projection);
        triton.draw(deltaTime.count() * speedMultiplier, projection);

        pluto.draw(deltaTime.count() * speedMultiplier, projection);


        asteroidBelt.draw(projection, asteroidProgram);
        kuiperBelt.draw(projection, kuiperProgram);
        oortCloud.draw(projection, oortCloudProgram);

        if (orbitsPresent) {
            drawOrbits(projection, orbitProgram, mercury, venus, earth, mars, jupiter, saturn, uranus, neptune, pluto);
        }


        mouseHoverDetection(window, screenWidth, screenHeight, zoomLevel, sun, mercury, earth, venus, mars, jupiter, saturn, uranus, neptune, pluto, projection, textShaderProgram, Characters, triviaShaderProgram);
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
