#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include "SV68-2021-3D.h"



float speedMultiplier = 1.0;
double lastKeyPressTime = 0.0;

int screenWidth = 800, screenHeight = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);  // Kamera gleda sa strane
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
float fov = 45.0f;
bool firstMouse = true;


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
    //glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f); // Kamera je na Z = 5
    //glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);  // Gleda ka centru scene (Sunce)
    //glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Definiše šta je "gore" u svetu

    //return glm::lookAt(cameraPosition, cameraTarget, upVector);

    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 calculateProjectionMatrix(int screenWidth, int screenHeight) {
    //float fov = glm::radians(45.0f); // 45 stepeni vidnog polja
    //float aspectRatio = (float)screenWidth / (float)screenHeight;
    //float nearPlane = 0.1f;
    //float farPlane = 100.0f;

    //return glm::perspective(fov, aspectRatio, nearPlane, farPlane);

    return glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
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


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Obrnuto jer koordinate idu od gore na dole
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (fov >= 1.0f && fov <= 45.0f) fov -= yoffset;
    if (fov <= 1.0f) fov = 1.0f;
    if (fov >= 45.0f) fov = 45.0f;
}

//funkcija da proveri slucajne visestruke klikove
bool isOneClick(double& lastKeyPressTime) {
    double debounceDelay = 0.5;
    double currentTime = glfwGetTime(); // trenutno vreme (sekunde od pokretanja aplikacije)

    if (currentTime - lastKeyPressTime > debounceDelay || lastKeyPressTime == 0.0f) {
        lastKeyPressTime = currentTime; // update vreme poslednje obrade
        return false;
    }
    return true;
}

void processInput(GLFWwindow* window, float deltaTime) {
    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += cameraSpeed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= cameraSpeed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        std::cout << "Pritisnuli ste ESC. Sve se gasi....";
    }
    
    if ((glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) && speedMultiplier <= 20 && !isOneClick(lastKeyPressTime))
    {
        speedMultiplier += 5;
    }

    if ((glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) && speedMultiplier > 0 && !isOneClick(lastKeyPressTime))
    {
        speedMultiplier -= 5;
        if (speedMultiplier < 0) {
            speedMultiplier = 1;
        }
    }

    if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS))
    {
        speedMultiplier = 0;
    }
}



int main() {
    GLFWwindow* window = initializeOpenGL(screenWidth, screenHeight, "3D Suncev sistem");
    if (!window) return -1;

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetScrollCallback(window, scroll_callback);

    //===============================PROGRAMS=====================================
    GLuint sunProgram = createProgram("sun.vert", "sun.frag");
    GLuint planetProgram = createProgram("planet.vert", "planet.frag");
    GLuint moonProgram = createProgram("moon.vert", "moon.frag");
    //===============================TEXTURES=====================================
    //SUN
    GLuint sunTextureID = loadTexture("8k_sun.jpg");
    //PLANETS
    GLuint mercuryTextureID = loadTexture("2k_mercury.jpg");
    GLuint venusTextureID = loadTexture("2k_venus.jpg");
    GLuint earthTextureID = loadTexture("2k_earth_with_clouds.jpg");
    GLuint marsTextureID = loadTexture("2k_mars.jpg");
    GLuint jupiterTextureID = loadTexture("2k_jupiter.jpg");
    //MOONS
    GLuint moonTextureID = loadTexture("2k_moon.jpg");
    GLuint deimosTextureID = loadTexture("2k_deimos.jpg");
    GLuint phobosTextureID = loadTexture("2k_phobos.jpg");
    GLuint ioTextureID = loadTexture("2k_io.jpg");
    GLuint europaTextureID = loadTexture("2k_europa.jpg");
    GLuint ganymedeTextureID = loadTexture("2k_ganymede.jpg");
    GLuint callistoTextureID = loadTexture("2k_callisto.jpg");

    //===============================SPACE BODIES INITS=====================================
    //SUN
    Sun sun(1.0f, 36, 18);
    
    //MERCURY
    Planet mercury(0.3f, 36, 18, 35.0f, 40.0f, 1.5f); // Merkur
    
    //VENUS
    Planet venus(0.55f, 36, 18, 25.0f, 30.0f, 2.0f);  // Venera
    
    //EARTH
    Planet earth(0.5f, 36, 18, 30.0f, 30.0f, 3.0f);  // Zemlja
    Moon moon(earth, 0.2f, 36, 18, 20.0f, 50.0f, 0.5f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromEarth)
    
    //MARS
    Planet mars(0.4f, 36, 18, 25.0f, 25.0f, 4.0f);   // Mars
    Moon phobos(mars, 0.18f, 36, 18, 15.0f, 80.0f, 0.2f);  // Fobos - manji i bliži Marsu
    Moon deimos(mars, 0.15f, 36, 18, 10.0f, 40.0f, 0.5f);  // Deimos - veći i dalje od Marsa

    
    //JUPITER
    Planet jupiter(0.7f, 36, 18, 20.0f, 20.0f, 5.5f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSun)
    Moon io(jupiter, 0.2f, 36, 18, 15.0f, 150.0f, 0.8f);      // Io - blizu Jupitera, najbrži
    Moon europa(jupiter, 0.18f, 36, 18, 10.0f, 100.0f, 1.2f);   // Evropa - ledena površina
    Moon ganymede(jupiter, 0.23f, 36, 18, 8.0f, 70.0f, 1.4f);  // Ganimed - najveći mesec
    Moon callisto(jupiter, 0.21f, 36, 18, 5.0f, 40.0f, 1.6f);  // Kalisto - najudaljeniji

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 viewMatrix = calculateCameraMatrix();
        glm::mat4 projectionMatrix = calculateProjectionMatrix(screenWidth, screenHeight);

       
        processInput(window, deltaTime);



        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set background color to dark gray
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //[SPACE BODIES DRAWING]
        //SUN
        sun.Draw(sunProgram, sunTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos);
        
        //MERCURY
        mercury.Draw(planetProgram, mercuryTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        
        //VENUS
        venus.Draw(planetProgram, venusTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        
        //EARTH
        earth.Draw(planetProgram, earthTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        moon.Draw(moonProgram, moonTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        
        //MARS
        mars.Draw(planetProgram, marsTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        phobos.Draw(moonProgram, phobosTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        deimos.Draw(moonProgram, deimosTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        
        //JUPITER
        jupiter.Draw(planetProgram, jupiterTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        io.Draw(moonProgram, ioTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        europa.Draw(moonProgram, europaTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        ganymede.Draw(moonProgram, ganymedeTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        callisto.Draw(moonProgram, callistoTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);



        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
