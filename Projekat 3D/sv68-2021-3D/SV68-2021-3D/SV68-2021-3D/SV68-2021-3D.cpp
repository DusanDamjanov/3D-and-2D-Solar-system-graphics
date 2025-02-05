#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include "SV68-2021-3D.h"


glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);  // Kamera gleda sa strane
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f, pitch = 0.0f;  // Rotacija kamere (horizontalno i vertikalno)
float cameraSpeed = 0.05f; // Brzina kretanja
float sensitivity = 0.1f;  // Osetljivost misa
bool firstMouse = true;
double lastX = 400, lastY = 300;


void checkOpenGLError(const std::string& location) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::string errorMessage;
        switch (error) {
        case GL_INVALID_ENUM:
            errorMessage = "GL_INVALID_ENUM (1280) - Nevalidan parametar za OpenGL funkciju.";
            break;
        case GL_INVALID_VALUE:
            errorMessage = "GL_INVALID_VALUE (1281) - Parametar funkcije je izvan dozvoljenog opsega.";
            break;
        case GL_INVALID_OPERATION:
            errorMessage = "GL_INVALID_OPERATION (1282) - OpenGL operacija nije validna u trenutnom stanju.";
            break;
        case GL_STACK_OVERFLOW:
            errorMessage = "GL_STACK_OVERFLOW (1283) - OpenGL stek je prekoračen.";
            break;
        case GL_STACK_UNDERFLOW:
            errorMessage = "GL_STACK_UNDERFLOW (1284) - OpenGL stek je ispod minimalnog nivoa.";
            break;
        case GL_OUT_OF_MEMORY:
            errorMessage = "GL_OUT_OF_MEMORY (1285) - OpenGL je ostao bez memorije.";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errorMessage = "GL_INVALID_FRAMEBUFFER_OPERATION (1286) - Framebuffer nije kompletan za render.";
            break;
        default:
            errorMessage = "Nepoznata greska: " + std::to_string(error);
            break;
        }
        std::cerr << "OpenGL Error at " << location << ": " << errorMessage << std::endl;
    }
}

GLFWwindow* initializeOpenGL(int width, int height, const char* title) {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    //if (glewInit() != GLEW_OK) 
    //    std::cerr << "GLEW initialization failed!" << std::endl;
    //    return nullptr;
    //}
    //checkOpenGLError("After glew init");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed!" << std::endl;
        return nullptr;
    }
    checkOpenGLError("After glad init");


    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    //glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
        if (!str.empty()) {
            std::cerr << "Greška pri linkovanju programa: " << infoLog << std::endl;
        }
    }
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    return program;
}



glm::mat4 calculateCameraMatrix() {
    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f); // Kamera je na Z = 5
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // Gleda ka centru scene (Sunce)
    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Definiše šta je "gore" u svetu

    return glm::lookAt(cameraPosition, cameraTarget, upVector);
}

glm::mat4 calculateProjectionMatrix(int screenWidth, int screenHeight) {
    float fov = glm::radians(45.0f); // 45 stepeni vidnog polja
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}

// Funkcija za učitavanje teksture
GLuint loadTexture(const char* filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Postavke teksture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Učitavanje slike
    stbi_set_flip_vertically_on_load(true); // Flipa teksturu ako je potrebno
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        std::cout << "Texture Loaded Successfully: " << filePath << std::endl;
        std::cout << "Width: " << width << ", Height: " << height << ", Channels: " << nrChannels << std::endl;
    }
    else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return 0;
    }

    return textureID;
}







int main() {
    int screenWidth = 800, screenHeight = 600;
    GLFWwindow* window = initializeOpenGL(screenWidth, screenHeight, "3D Suncev sistem");
    if (!window) return -1;

    GLuint sunProgram = createProgram("sun.vert", "sun.frag");
    GLuint planetProgram = createProgram("planet.vert", "planet.frag");

    GLuint sunTextureID = loadTexture("8k_sun.jpg");
    GLuint jupiterTextureID = loadTexture("2k_jupiter.jpg");

    Sun sun(1.0f, 36, 18);
    Planet jupiter(0.7f, 36, 18, 20.0f, 20.0f, 4.0f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSun)


    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 viewMatrix = calculateCameraMatrix();
        glm::mat4 projectionMatrix = calculateProjectionMatrix(screenWidth, screenHeight);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
            std::cout << "Pritisnuli ste ESC. Sve se gasi....";
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set background color to dark gray
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // **Draw Sphere**
        sun.Draw(sunProgram, sunTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos);

        // Crtanje Jupitera
        jupiter.Draw(planetProgram, jupiterTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
