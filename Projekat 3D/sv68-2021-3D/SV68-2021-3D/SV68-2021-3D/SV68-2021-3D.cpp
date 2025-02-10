#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include "SV68-2021-3D.h"



float speedMultiplier = 1.0;
double lastKeyPressTime = 0.0;

int screenWidth = 1600, screenHeight = 800;

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed!" << std::endl;
        return nullptr;
    }
    checkOpenGLError("After glad init");


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

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
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

glm::mat4 calculateProjectionMatrix(int screenWidth, int screenHeight) {
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

        //std::cout << "Texture Loaded Successfully: " << filePath << std::endl;
        //std::cout << "Width: " << width << ", Height: " << height << ", Channels: " << nrChannels << std::endl;
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

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Standardno punjenje poligona
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Samo ivice
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); // Samo tjemena
    }
}


//render fja za details prikaz planete
void renderInfoBox(float x, float y, float width, float height, GLuint shaderProgram, const char* textureName) {
    GLuint texture = loadTexture(textureName);

    if (texture == 0) {
        std::cerr << "Error: Nevalidni Texture ID" << std::endl;
        return;
    }

    // Postavi ortografsku projekciju za prikazivanje informacija
    glm::mat4 orthoProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjection));


    // Prilagodimo visinu i širinu da budu u NDC prostoru
    width = width * 2.0f;
    height = height * 2.0f;

    // Verteksi za pravougaonik (prikazuje se u ekranskom prostoru)
    float vertices[6][4] = {
        {x, y, 0.0f, 1.0f},  
        {x, y - height, 0.0f, 0.0f}, 
        {x + width, y - height, 1.0f, 0.0f},

        {x, y, 0.0f, 1.0f},  
        {x + width, y - height, 1.0f, 0.0f},
        {x + width, y, 1.0f, 1.0f} 
    };


    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

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

void shouldShowDetails(GLuint shaderProgram, Sun& sun, std::unordered_map<std::string, Moon*> moons, 
    std::unordered_map<std::string, Planet*> planets) {

    float minDistance = 0.2f;

    if (glm::distance(cameraPos, sun.getPosition()) < (sun.getRadius() + minDistance)) {
        glDisable(GL_DEPTH_TEST);
        renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, shaderProgram, "sun-trivia.png");
        glEnable(GL_DEPTH_TEST);
        return;
    }
    
    for (const auto& pair : moons) {
        const std::string& moonName = pair.first; 
        Moon& moon = *pair.second;             
        
        if (glm::distance(cameraPos, moon.getPosition()) < (moon.getRadius() + minDistance)) {
            std::string triviaPathStr = moonName + "-trivia.png";
            const char* triviaPath = triviaPathStr.c_str();       

            glDisable(GL_DEPTH_TEST);
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, shaderProgram, triviaPath);
            glEnable(GL_DEPTH_TEST);
            return;
        }
    }

    for (const auto& pair : planets) {
        const std::string& planetName = pair.first; 
        Planet& planet = *pair.second;             
        
        if (glm::distance(cameraPos, planet.getPosition()) < (planet.getRadius() + minDistance)) {
            std::string triviaPathStr = planetName + "-trivia.png";
            const char* triviaPath = triviaPathStr.c_str();       

            glDisable(GL_DEPTH_TEST);
            renderInfoBox(-0.95f, 0.9f, 0.4f, 0.2f, shaderProgram, triviaPath);
            glEnable(GL_DEPTH_TEST);
            return;
        }
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
    GLuint ringProgram = createProgram("ring.vert", "ring.frag");
    GLuint triviaShaderProgram = createProgram("details.vert", "details.frag");


    //===============================TEXTURES=====================================
    //PLANETS
    GLuint sunTextureID = loadTexture("sun-tex.jpg");
    GLuint mercuryTextureID = loadTexture("mercury-tex.jpg");
    GLuint venusTextureID = loadTexture("venus-tex.jpg");
    GLuint earthTextureID = loadTexture("earth-tex.jpg");
    GLuint marsTextureID = loadTexture("mars-tex.jpg");
    GLuint jupiterTextureID = loadTexture("jupiter-tex.jpg");
    GLuint saturnTextureID = loadTexture("saturn-tex.jpg");
    GLuint ringTextureID = loadTexture("saturn-ring-tex.jpg");
    GLuint uranusTextureID = loadTexture("uranus-tex.jpg");
    GLuint plutoTextureID = loadTexture("pluto-tex.jpg");
    GLuint neptuneTextureID = loadTexture("neptune-tex.jpg");

    //MOONS
    GLuint moonTextureID = loadTexture("moon-tex.jpg");
    GLuint deimosTextureID = loadTexture("deimos-tex.jpg");
    GLuint phobosTextureID = loadTexture("phobos-tex.jpg");
    GLuint ioTextureID = loadTexture("io-tex.jpg");
    GLuint europaTextureID = loadTexture("europa-tex.jpg");
    GLuint ganymedeTextureID = loadTexture("ganymede-tex.jpg");
    GLuint callistoTextureID = loadTexture("callisto-tex.jpg");
    GLuint titanTextureID = loadTexture("titan-tex.jpg");
    GLuint rheaTextureID = loadTexture("rhea-tex.jpg");
    GLuint iapetusTextureID = loadTexture("iapetus-tex.jpg");
    GLuint umbrielTextureID = loadTexture("umbriel-tex.jpg");
    GLuint arielTextureID = loadTexture("ariel-tex.jpg");
    GLuint mirandaTextureID = loadTexture("miranda-tex.jpg");
    GLuint tritonTextureID = loadTexture("triton-tex.jpg");


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

    //SATURN
    Planet saturn(0.65f, 36, 18, 18.0f, 18.0f, 8.5f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSun)
    SaturnRing ring(100, 0.6f, 1.0f);
    Moon titan(saturn, 0.27f, 36, 18, 10.0f, 50.0f, 0.8f);   // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSaturn)
    Moon rhea(saturn, 0.2f, 36, 18, 8.0f, 40.0f, 1.2f);    
    Moon iapetus(saturn, 0.19f, 36, 18, 6.0f, 30.0f, 1.6f); 

    //URANUS
    Planet uranus(0.55f, 36, 18, 17.0f, 15.0f, 10.0f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSun)
    Moon umbriel(uranus, 0.22f, 36, 18, 6.0f, 35.0f, 0.8f);  // Umbriel - tamna površina
    Moon ariel(uranus, 0.2f, 36, 18, 5.0f, 30.0f, 0.5f);    // Ariel - ledena površina
    Moon miranda(uranus, 0.2f, 36, 18, 5.0f, 30.0f, 1.1f);    // Ariel - ledena površina

    //PLUTO
    Planet pluto(0.25f, 36, 18, 10.0f, 10.0f, 12.0f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSun)

    //NEPTUNE
    Planet neptune(0.50f, 36, 18, 16.0f, 14.0f, 13.0f); // (radius, sectors, stacks, rotationSpeed, orbitSpeed, distanceFromSun)
    Moon triton(neptune, 0.22f, 36, 18, 9.0f, 55.0f, 0.5f);   // Triton - najveći mesec


    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(window)) {
       
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glm::mat4 viewMatrix = calculateCameraMatrix();
        glm::mat4 projectionMatrix = calculateProjectionMatrix(screenWidth, screenHeight);

        processInput(window, deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
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

        //SATURN
        saturn.Draw(planetProgram, saturnTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        ring.Draw(ringProgram, ringTextureID, viewMatrix, projectionMatrix, saturn.getPosition());
        titan.Draw(moonProgram, titanTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        rhea.Draw(moonProgram, rheaTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        iapetus.Draw(moonProgram, iapetusTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);

        //URANUS
        uranus.Draw(planetProgram, uranusTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        umbriel.Draw(moonProgram, umbrielTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        ariel.Draw(moonProgram, arielTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        miranda.Draw(moonProgram, mirandaTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);
        
        //PLUTO
        pluto.Draw(planetProgram, plutoTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);

        //NEPTUNE
        neptune.Draw(planetProgram, neptuneTextureID, viewMatrix, projectionMatrix, deltaTime, cameraPos, speedMultiplier);
        triton.Draw(moonProgram, tritonTextureID, viewMatrix, projectionMatrix, deltaTime, speedMultiplier);

        std::unordered_map<std::string, Planet*> planets = {
            {"mercury", &mercury},
            {"venus", &venus},
            {"earth", &earth},
            {"mars", &mars},
            {"jupiter", &jupiter},
            {"saturn", &saturn},
            {"uranus", &uranus},
            {"neptune", &neptune},
            {"pluto", &pluto}
        };

        std::unordered_map<std::string, Moon*> moons = {
            {"moon", &moon},               // Mesec Zemlje
            {"deimos", &deimos},           // Mesec Marsa
            {"phobos", &phobos},           // Mesec Marsa
            {"io", &io},                   // Mesec Jupitera
            {"europa", &europa},           // Mesec Jupitera
            {"ganymede", &ganymede},       // Mesec Jupitera
            {"callisto", &callisto},       // Mesec Jupitera
            {"titan", &titan},             // Mesec Saturna
            {"rhea", &rhea},               // Mesec Saturna
            {"iapetus", &iapetus},         // Mesec Saturna
            {"umbriel", &umbriel},         // Mesec Urana
            {"ariel", &ariel},             // Mesec Urana
            {"miranda", &miranda},         // Mesec Urana
            {"triton", &triton}            // Mesec Neptuna
        };

        shouldShowDetails(triviaShaderProgram, sun, moons, planets);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


