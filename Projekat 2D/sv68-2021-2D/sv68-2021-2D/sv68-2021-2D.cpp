#define _USE_MATH_DEFINES
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Za funkcije za transformaciju matrica
#include <glm/gtc/type_ptr.hpp>        // Za pretvaranje matrica u pokazivače


GLFWwindow* initializeOpenGL(int width, int height, const char* title) {
    // Inicijalizacija GLFW-a
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return nullptr;
    }

    // Postavljanje verzije OpenGL-a na 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Kreiranje prozora
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    // Postavljanje OpenGL konteksta
    glfwMakeContextCurrent(window);

    // Inicijalizacija GLEW-a
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW initialization failed!" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    // Podešavanje OpenGL viewport-a
    glViewport(0, 0, width, height);

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

    // Proveri kompajliranje vertex šejdera za Saturn
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Greška u vertex šejderu za Saturn: " << infoLog << std::endl;
    }

    // Proveri kompajliranje fragment šejdera za Saturn
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Greška u fragment šejderu za Saturn: " << infoLog << std::endl;
    }

    
    // Proveri linkovanje programa za Saturn
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::string str(infoLog);
        if (!str.empty()){
            std::cerr << "Greška pri linkovanju programa za Saturn: " << infoLog << std::endl;
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

//Funkcija za izracunavanje projekcije
glm::mat4 calculateProjection(int screenWidth, int screenHeight, float zoomLevel, float offsetX, float offsetY) {
    // Izračunavanje aspect ratio-a ekrana
    float aspectRatio = static_cast<float>(screenWidth) / screenHeight;

    // Obrnuti faktor zooma (veći zoomLevel = bliže)
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


// Klasa za Sunce
class Sun2D {
public:
    float x, y;         // Pozicija Sunca
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

    // Ažuriranje ugla rotacije
    void update(float deltaTime) {
        currentAngle += rotationSpeed * deltaTime;
        if (currentAngle > 360.0f) currentAngle -= 360.0f;
    }

    // Generišemo podatke o tačkama kruga
    void generateCircleData() {
        int numSegments = 100; // Broj segmenata kruga (veća vrednost = glatkiji krug)
        GLfloat* vertices = new GLfloat[(numSegments + 2) * 4]; // +2 za centar i poslednju tačku

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
            std::cerr << "Failed to generate VAO!" << std::endl;
            delete[] vertices;
            return;
        }
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        if (VBO == 0) {
            std::cerr << "Failed to generate VBO!" << std::endl;
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);

        GLenum error1 = glGetError();
        if (error1 != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error1 << std::endl;
        }

        //Uniform za teksturu
        GLuint textureLoc = glGetUniformLocation(shaderProgram, "sunTexture");
        glUniform1i(textureLoc, 0); // Koristi teksturnu jedinicu 0


        // Uniform za projekciju
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Uniform za rotaciju
        glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(currentAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 102); // 102 [ 100 segmenata + centar + zatvaranje ]
        GLenum error2 = glGetError();
        if (error2 != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << error2 << std::endl;
        }

        glBindVertexArray(0);
    }

    glm::vec2 getScreenPosition() {
        // Računanje trenutne pozicije Sunca uzimajući u obzir offsete
        return glm::vec2(x, y);
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

//klasa za Planete
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
        // Izračunavanje polumanje ose na osnovu ekscentričnosti
        semiMinorAxis = distance * sqrt(1 - eccentricity * eccentricity);
        textureID = loadTexture(texturePath); // Učitavanje teksture
        generateCircleData();
        generateOrbitData();
        float scalingFactor = std::max(0.5f, std::min(1.5f / distance, 1.0f));
        size *= scalingFactor;
    }

    // Ažuriranje orbite i rotacije
    void update(float deltaTime) {
        currentOrbit += orbitSpeed * deltaTime;
        if (currentOrbit > 360.0f) currentOrbit -= 360.0f;

        // Ažuriranje rotacije oko svoje ose
        currentSelfRotation += selfRotationSpeed * deltaTime;
        if (currentSelfRotation > 360.0f) currentSelfRotation -= 360.0f;
    }

    // Generisanje podataka o krugu
    void generateCircleData() {
        int numSegments = 50; // Manje segmenata za manju planetu
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

    // Generisanje podataka za orbitu
    void generateOrbitData() {
        int numSegments = 100; // Više segmenata za glatku orbitu
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

        // Transformacija za planetu (orbita + rotacija oko svoje ose)
        float angle = currentOrbit * M_PI / 180.0f;
        glm::vec2 position(cos(angle) * distance - distance * eccentricity, sin(angle) * semiMinorAxis);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
        transform = glm::rotate(transform, glm::radians(currentSelfRotation), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotacija oko ose

        GLuint transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 52); // 50 segmenata + centar + zatvaranje
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
    glm::vec2 getPosition() {
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
        // Računaj trenutnu poziciju Sunca uzimajući u obzir offsete
        glm::vec2 sunScreenPos = getPosition();

        // Skaliraj veličinu Sunca prema zoom nivou
        float scaledSize = size * zoomLevel;

        // Vrati centar i radijus kao strukturu
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

class OortCloud {
public:
    int numObjects;        // Broj objekata u oblaku
    float innerRadius;     // Unutrašnji radijus oblaka
    float outerRadius;     // Spoljašnji radijus oblaka
    GLuint VAO, VBO;       // OpenGL objekti za crtanje

    OortCloud(int num, float innerR, float outerR)
        : numObjects(num), innerRadius(innerR), outerRadius(outerR) {
        generateCloudData();
    }

    void generateCloudData() {
        std::vector<GLfloat> vertices;
        vertices.reserve(numObjects * 3); // X, Y, Z za svaki objekat

        for (int i = 0; i < numObjects; ++i) {
            // Nasumične sfere koordinata
            float radius = innerRadius + static_cast<float>(rand()) / RAND_MAX * (outerRadius - innerRadius);
            float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI; // Ugao u XY ravni
            float phi = acos(1.0f - 2.0f * static_cast<float>(rand()) / RAND_MAX); // Ugao od Z ose

            // Konverzija u kartezijanske koordinate
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }

        // OpenGL priprema
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void draw(const glm::mat4& projection, GLuint shaderProgram) {
        glUseProgram(shaderProgram);

        // Uniform za projekciju
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        if (projectionLoc != -1) {
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, numObjects); // Crtaj tačke
        glBindVertexArray(0);
    }
};

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

bool isOneClick(double& lastKeyPressTime) {
    double debounceDelay = 0.5;
    double currentTime = glfwGetTime(); // Trenutno vreme (sekunde od pokretanja aplikacije)

    if (currentTime - lastKeyPressTime > debounceDelay) {
        lastKeyPressTime = currentTime; // Ažuriraj vreme poslednje obrade
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

    float scaledRadius = objectPos.radius * zoomLevel;


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

    float scaledRadius = objectPos.radius * zoomLevel;


    if (dx * dx + dy * dy <= objectPos.radius * objectPos.radius) {
        return true;
    }
    else {
        return false;
    }
}


void mouseHoverDetection(GLFWwindow* window, int screenWidth, int screenHeight, float zoomLevel, Sun2D& sun, Planet2D& earth, Planet2D & venus,
    Planet2D& mars, Planet2D& jupiter, Planet2D& saturn, Planet2D& uranus, Planet2D& neptune, Planet2D& pluto, glm::mat4 projection) {
    glm::vec2 mouseWorldPos = getMouseWorldPosition(window, screenWidth, screenHeight, projection);
    
    //Proveri za sunce
    Sun2D::SunBounds sunBounds = sun.getSunBounds(zoomLevel);
    if (isMouseOverSun(mouseWorldPos, sunBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    // Proveri za planete
    Planet2D::PlanetBounds earthBounds = earth.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, earthBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds venusBounds = venus.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, venusBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds marsBounds = mars.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, marsBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds jupiterBounds = jupiter.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, jupiterBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds saturnBounds = saturn.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, saturnBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds uranusBounds = uranus.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, uranusBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds neptuneBounds = neptune.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, neptuneBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }
    Planet2D::PlanetBounds plutoBounds = pluto.getPlanetBounds(zoomLevel);
    if (isMouseOverPlanet(mouseWorldPos, plutoBounds, zoomLevel)) {
        std::cout << "iznad je" << "\n";
    }
    else {
        std::cout << "nije" << "\n";
    }

}


void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color, GLuint textShaderProgram) {
    // Aktiviraj šejder za tekst
    glUseProgram(textShaderProgram);

    // Setuj uniforme za boju teksta
    GLint colorLoc = glGetUniformLocation(textShaderProgram, "textColor");
    glUniform3f(colorLoc, color.x, color.y, color.z);

    // Postavi poziciju i skaliranje teksta
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(scale, scale, 1.0f));
    GLint modelLoc = glGetUniformLocation(textShaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Renderuj tekst ovde... (npr. kroz VAO/VBO za fontove)
}



int main() {
    int screenWidth = 1800, screenHeight = 950;

    GLFWwindow* window = initializeOpenGL(screenWidth, screenHeight, "Suncev Sistem - 2D");
    if (!window) return -1;

    double lastClickTime = glfwGetTime();
    float zoomLevel = 1.0f; // Početni nivo zooma
    float minZoom = 5.0f;   //kolko maximalno mozes da umanjis
    float maxZoom = 0.2f;   //kolko maximalno mozes da uvecas
    float offsetX = 0.0f; // Ofset za horizontalno pomeranje
    float offsetY = 0.0f; // Ofset za vertikalno pomeranje
    float panSpeed = 0.05f; //brzina pomeranja pogleda

    bool orbitsPresent = true;  //boolean da li ce orbite biti prikazane

    float speedMultiplier = 1.0f;    // Faktor ubrzanja animacije


    // Kreiranje ortografske projekcije
    float aspectRatio = static_cast<float>(screenWidth) / screenHeight;

    // Kreiranje ortografske projekcije koja koristi dimenzije ekrana
    glm::mat4 projection = glm::ortho(
        -screenWidth / 2.0f * zoomLevel,  // Leva granica
        screenWidth / 2.0f * zoomLevel,  // Desna granica
        -screenHeight / 2.0f * zoomLevel, // Donja granica
        screenHeight / 2.0f * zoomLevel, // Gornja granica
        -1.0f,  // Z-blizina
        1.0f   // Z-daljina
    );

    //ucitavanje svih sejdera za planete
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
    GLuint kuiperProgram = createProgram("kuiper.vert", "kuiper.frag");
    GLuint oortCloudProgram = createProgram("oortCloud.vert", "oortCloud.frag");


    // Kreiranje planeta     //TODO: MOZEMO NAMESTITI DA I SUNCE POSTANE KLASA PLANET2D --> PROBAJ (al nisam siguran da ce moci ipak jer ovo je centralni objekat u prici) !!
    Sun2D sun(0.0f, 0.0f, 0.05f, 90.0f, sunProgram, "sun-texture.jpg"); // Smanjeno Sunce

    Planet2D mercury(0.1f, 0.206f, 0.02f, 150.0f, mercuryProgram, "mercury-texture.jpg", 30.0f); // Povećan Merkur
    Planet2D venus(0.15f, 0.007f, 0.03f, 120.0f, venusProgram, "venus-texture.jpeg", 30.0f); // Venera
    Planet2D earth(0.2f, 0.017f, 0.035f, 100.0f, earthProgram, "earth-texture.jpg", 30.0f); // Zemlja
    Planet2D mars(0.3f, 0.093f, 0.03f, 80.0f, marsProgram, "mars-texture.jpg", 30.0f); // Mars
    Planet2D jupiter(0.5f, 0.049f, 0.06f, 40.0f, jupiterProgram, "jupiter-texture.jpg", 30.0f); // Jupiter
    Planet2D saturn(0.7f, 0.056f, 0.05f, 30.0f, saturnProgram, "saturn-texture.jpg", 30.0f); // Saturn
    Planet2D uranus(1.0f, 0.046f, 0.04f, 20.0f, uranusProgram, "uranus-texture.jpg", 30.0f); // Uran
    Planet2D neptune(1.3f, 0.010f, 0.04f, 10.0f, neptuneProgram, "neptune-texture.jpg", 30.0f); // Neptun
    Planet2D pluto(1.6f, 0.248f, 0.02f, 5.0f, plutoProgram, "pluto-texture.jpg", 30.0f); // Pluton

    Moon2D moon(earth, 0.03f, 0.01f, 300.0f, moonProgram, "moon-texture.jpg"); // Mesec oko Zemlje
    Moon2D phobos(mars, 0.06f, 0.008f, 300.0f, moonProgram, "phobos-texture.jpg"); // Fobos - Mars
    Moon2D deimos(mars, 0.15f, 0.01f, 150.0f, moonProgram, "deimos-texture.jpg");   // Deimos - Mars
    Moon2D io(jupiter, 0.1f, 0.015f, 250.0f, moonProgram, "io-texture.jpg");       // Io - Jupiter
    Moon2D europa(jupiter, 0.15f, 0.012f, 200.0f, moonProgram, "europa-texture.jpg");    // Evropa - Jupiter
    Moon2D ganymede(jupiter, 0.25f, 0.02f, 150.0f, moonProgram, "ganymede-texture.jpg");   // Ganimed - Jupiter
    Moon2D callisto(jupiter, 0.35f, 0.018f, 100.0f, moonProgram, "callisto-texture.jpg");   // Kalisto - Jupiter
    Moon2D titan(saturn, 0.15f, 0.03f, 200.0f, moonProgram, "titan-texture.jpg");       // Titan - Saturn
    Moon2D rhea(saturn, 0.25f, 0.02f, 150.0f, moonProgram, "rhea-texture.jpg");       // Rea - Saturn
    Moon2D iapetus(saturn, 0.4f, 0.015f, 100.0f, moonProgram, "iapetus-texture.jpg");     // Japet - Saturn
    Moon2D miranda(uranus, 0.1f, 0.02f, 180.0f, moonProgram, "miranda-texture.jpg");     // Miranda - Uran
    Moon2D ariel(uranus, 0.2f, 0.025f, 140.0f, moonProgram, "ariel-texture.jpg");       // Ariel -  Uran
    Moon2D umbriel(uranus, 0.3f, 0.02f, 100.0f, moonProgram, "umbriel-texture.jpg");       // Umbriel - Uran
    Moon2D triton(neptune, 0.15f, 0.03f, 120.0f, moonProgram, "triton-texture.jpg"); // Triton - Neptun
    
    AsteroidBelt asteroidBelt(1000, 0.35f, 0.45f); // 1000 asteroida između Marsa i Jupitera
    AsteroidBelt kuiperBelt(1500, 1.5f, 2.0f);     // 1500 objekata između Neptuna i Plutona
    OortCloud oortCloud(5000, 1.8f, 2.5f); 




    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        glm::mat4 projection = calculateProjection(screenWidth, screenHeight, zoomLevel, offsetX, offsetY);

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            mouseHoverDetection(window, screenWidth, screenHeight, zoomLevel, sun, earth, venus, mars, jupiter, saturn, uranus, neptune, pluto, projection);
        }

        //ZUMIRANJE
        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            zoomLevel += 0.01f; // Zumiraj
            if (zoomLevel < maxZoom) zoomLevel = maxZoom; // Minimalni zoom
            std::cout << "zoom level nakon zumiranja: " << zoomLevel <<"\n";
        }

        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            zoomLevel -= 0.01f; // Odzumiraj
            if (zoomLevel > minZoom) zoomLevel = minZoom; // Maksimalni zoom
            std::cout << "zoom level nakon odzumiranja: " << zoomLevel << "\n";
        }

        //GASENJE
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            std::cout << "Pritisnuli ste ESCAPE i sve gasim...";
            exit(0);
        }

        //POMERANJE PO EKRANU
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            offsetY += panSpeed * zoomLevel; // Pomeranje nagore
            std::cout << "Offset y" << offsetY << "\n";
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            offsetY -= panSpeed * zoomLevel; // Pomeranje nadole
            std::cout << "Offset y" << offsetY << "\n";
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            offsetX -= panSpeed * zoomLevel; // Pomeranje ulevo
            std::cout << "Offset x" << offsetX << "\n";

        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            offsetX += panSpeed * zoomLevel; // Pomeranje udesno
            std::cout << "Offset x" << offsetX << "\n";

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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
