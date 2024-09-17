#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <../include/textures/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <shaders/shader.h>
#include <camera/camera.h>
#include <vector>
#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double yps);
void processInput(GLFWwindow *window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int createCube();
unsigned int generateTexture(const  char* texturePath, Shader &ourShader);
void renderMenu(Shader &menuShader, unsigned int VAO);
unsigned int createMenuQuad();
void renderCube(Shader &cubeShader, unsigned int VAO, glm::vec3 cubePositions[], bool isNegative);
void renderButton(Shader &buttonShader, unsigned int VAO);
unsigned int createButton();
unsigned int createRectangle(float vertices[], unsigned int sizeOfVertices);
void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VAO, unsigned int VBO);
void initalizeTextRendering(FT_Library &ft, FT_Face &face);
std::vector<unsigned int> createTextQuad();
std::string GetButtonPosAsString();
std::string GetCursorPosAsString();
void updateButtonPositionOnResize(int widthOffset, int heightOffset);
void createVaoVbo(GLuint &vaoID, GLuint &vboID, unsigned int sizeOfVertices, unsigned int stride, unsigned int offset, GLenum usageType);
struct Button;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
unsigned int NEW_HEIGHT;
unsigned int NEW_WIDTH;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

float menuLastX = SCR_WIDTH / 2.0f;
float menuLastY = SCR_HEIGHT / 2.0f;

float menuWidth = 100.0f;
float buttonWidth = 60.0f;
float buttonHeight = 50.0f;

float delaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
bool hasOpenedMenu = false;
bool menuKeyPressed = false;
bool isPaused = false;
float cubeRotations[10] = {0.0f};
std::vector<Button> buttonPositions;

bool inButton = false;
bool buttonPressed = false;
bool isNegative = false;

bool xPressed = false;
bool coordinatesOn = false;


enum UIElementType {
    BUTTON,
    MENU
};

struct Button
{
    glm::vec2 topLeft;
    glm::vec2 topRight;
    glm::vec2 bottomLeft;
    glm::vec2 bottomRight;
};

struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size; // Size of glyph
    glm::ivec2 Bearing; // Offset from baseline left/top of glyph
    unsigned int Advance; // Offset to advance to next glyph
};

struct UIData {
    GLuint buttonVBO;
    GLuint menuVBO;
} uiData;

std::map<char, Character> Characters;
void updateUI(UIElementType type, GLuint vboID, int screenWidth, int screenHeight);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    // build and compile our shader zprogram
    // ------------------------------------
    Shader cubeShader("../include/shaders/cube_shader.vs", "../include/shaders/cube_shader.fs");

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };


    // need to refactor
    // ------------------------------------------------------------------------
    // Ensure OpenGL context is active before these calls
    // If using a windowing library like GLFW or SDL, ensure the window has been created and made active
    if (!glfwGetCurrentContext()) { 
        std::cout << "OpenGL context not initialized!" << std::endl;
    }
    unsigned int cubeVAO = createCube();
    unsigned int texture1 = generateTexture("../images/container.jpg", cubeShader);

    Shader menuShader("../include/shaders/menu_shader.vs", "../include/shaders/menu_shader.fs");
    GLuint menuVAO, menuVBO;
    createVaoVbo(menuVAO, menuVBO, 30, 5, 3, GL_DYNAMIC_DRAW);
    std::cout << "Menu VAO: " << menuVAO << ", Menu VBO: " << menuVBO;
    menuShader.use();
    menuShader.setVec3("color", glm::vec3(1, 1.0f, 1.0f));
    
    Shader buttonShader("../include/shaders/menu_shader.vs", "../include/shaders/menu_shader.fs");
    GLuint buttonVAO, buttonVBO;
    createVaoVbo(buttonVAO, buttonVBO, 30, 5, 3, GL_DYNAMIC_DRAW);
    std::cout << "Button VAO: " << buttonVAO << ", Button VBO: " << buttonVBO;
    buttonShader.use();
    buttonShader.setVec3("color", glm::vec3(0.0, 0.0f, 0.0f));

    uiData.menuVBO = menuVBO;
    uiData.buttonVBO = buttonVBO;

    updateUI(MENU, menuVBO, SCR_WIDTH, SCR_HEIGHT);
    updateUI(BUTTON, buttonVBO, SCR_WIDTH, SCR_HEIGHT);
    
    // ------------------------------------------------------------------------------------------
    
    // Loading and dealing with text
    // -----------------------------
    Shader textShader("../include/shaders/text.vs", "../include/shaders/text.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    FT_Library ft;
    FT_Face face;

    if(FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init free type libary" << std::endl;
        return -1;
    }
    if(FT_New_Face(ft, "../include/fonts/arial.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    if(FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        std::cout << "ERROR::FREETYPE: Failed to load Glypth" << std::endl;
        return -1;
    }

    initalizeTextRendering(ft, face);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    std::vector<unsigned int> textVaoVbo = createTextQuad();
    unsigned int textVAO = textVaoVbo[0];
    unsigned int textVBO = textVaoVbo[1];

    std::vector<unsigned int> buttonPosVaoVbo = createTextQuad();
    unsigned int buttonPosVAO = buttonPosVaoVbo[0];
    unsigned int buttonPosVBO = buttonPosVaoVbo[1];


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        delaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear the depth buffer now!

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);

        // activate shader
        cubeShader.use();

        // pass projection to shader(note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        cubeShader.setMat4("projection", projection);

        // cameera/view transformation
        glm::mat4 view = camera.GetViewMatrix();
        cubeShader.setMat4("view", view);

        // render box
        glBindVertexArray(cubeVAO);

        renderCube(cubeShader, cubeVAO, cubePositions, isNegative);
        
        if(hasOpenedMenu)
        {
          
            menuShader.use();
            renderMenu(menuShader, menuVAO);

            buttonShader.use();
            renderButton(buttonShader, buttonVAO);

            textShader.use();
            RenderText(textShader, GetCursorPosAsString() , 150.0f, 550.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f), textVAO, textVBO);
            RenderText(textShader, GetButtonPosAsString() , 150.0f, 500.0f, 0.3f, glm::vec3(0.5, 0.8f, 0.2f), buttonPosVAO, buttonPosVBO);
        
        }

        //renderButton(buttonShader, buttonVAO);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glFinish();
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // -----------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);

    NEW_WIDTH = width;
    NEW_HEIGHT = height;

    updateUI(MENU, uiData.menuVBO, width, height);
    updateUI(BUTTON, uiData.buttonVBO, width, height);

}



void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xPos = static_cast<float>(xposIn);
    float yPos = static_cast<float>(yposIn);
    

    // Initialize lastX and lastY only once
    if(firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    float yPosFlpped = static_cast<float>(NEW_HEIGHT) - yPos;
     // Handle menu state
    if(hasOpenedMenu)
    {
        menuLastX = xPos;
        menuLastY = yPosFlpped;
        for(const Button& button : buttonPositions)
        {
            std::cout << coordinatesOn;
            if(coordinatesOn)
            {
                std::cout << "Mouse xPos: " << menuLastX << ", Mouse yPos: " << menuLastY << "| Button topLeft X: " << button.topLeft.x <<
                ", Button topRight X: " << button.topRight.x << ", Button bottomLeft Y: " << button.bottomLeft.y << 
                ", Button topLeft Y: " << button.topLeft.y;

                //coordinatesOn = false;
            }
            if((menuLastX > button.topLeft.x && xPos < button.topRight.x) && (menuLastY > button.bottomLeft.y && yPos < button.topLeft.y))
            {
                inButton = true;
            }
        }
        return;
    }
    else
    {
        inButton = false;
    }


    // Calculate offsets
    float xoffset = xPos - lastX;
    float yoffset = lastY - yPos;

    // Update lastX and lastY
    lastX = xPos;
    lastY = yPos;

    // Pass offsets to camera processing
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void renderMenu(Shader &menuShader, unsigned int VAO)
{

    glDisable(GL_DEPTH_TEST);
    // set up an orthographic projection for 2d rendering
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.f, static_cast<float>(SCR_HEIGHT));
    menuShader.use();
    menuShader.setMat4("projection", projection);

    // Draw the quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

}

void renderButton(Shader &buttonShader, unsigned int VAO)
{
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.f, static_cast<float>(SCR_HEIGHT));
    buttonShader.use();
    buttonShader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void renderMouseCoordinates(Shader &coordinatesShader, unsigned int VAO)
{

}

void renderCube(Shader &cubeShader, unsigned int VAO, glm::vec3 cubePositions[], bool isNegative)
{
    for(unsigned int i = 0 ; i < 10; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        if(!hasOpenedMenu)
        {
            if(isNegative)
            {
                cubeRotations[i] += 0.01f;
            }
            else
            {
                cubeRotations[i] -= 0.01f; 
            } 
        } 
        model = glm::rotate(model, cubeRotations[i], glm::vec3(1.0f, 0.3f, 0.5f)); 
        cubeShader.setMat4("model", model); glDrawArrays(GL_TRIANGLES, 0, 36); 
    } 

    glBindVertexArray(VAO); 
    glDrawArrays(GL_TRIANGLES, 0 ,36); 
    glBindVertexArray(0); 
}


void updateUI(UIElementType type, GLuint vboID, int screenWidth, int screenHeight) {
    float vertices[30]; // Array to hold vertex data

    // Variables for dimensions and positions
    float width, height, x, y;

    switch (type) {
        case BUTTON: {
            width = screenWidth * 0.1f;
            height = screenHeight * 0.1f;

            x = 0.0f;
            y = screenHeight - height;
            break;
        }
        case MENU: {
            width = screenWidth * 0.1f;
            height = screenHeight;

            x = 0.0f;
            y = 0.0f;
            break;
        }
    }

    // Define vertices and texture coordinates
    vertices[0] = x; vertices[1] = y; vertices[2] = 0.0f; // Bottom-left position
    vertices[3] = 0.0f; vertices[4] = 0.0f; // Bottom-left texture coordinate

    vertices[5] = x + width; vertices[6] = y; vertices[7] = 0.0f; // Bottom-right position
    vertices[8] = 1.0f; vertices[9] = 0.0f; // Bottom-right texture coordinate

    vertices[10] = x + width; vertices[11] = y + height; vertices[12] = 0.0f; // Top-right position
    vertices[13] = 1.0f; vertices[14] = 1.0f; // Top-right texture coordinate

    vertices[15] = x; vertices[16] = y; vertices[17] = 0.0f; // Bottom-left position
    vertices[18] = 0.0f; vertices[19] = 0.0f; // Bottom-left texture coordinate

    vertices[20] = x + width; vertices[21] = y + height; vertices[22] = 0.0f; // Top-right position
    vertices[23] = 1.0f; vertices[24] = 1.0f; // Top-right texture coordinate

    vertices[25] = x; vertices[26] = y + height; vertices[27] = 0.0f; // Top-left position
    vertices[28] = 0.0f; vertices[29] = 1.0f; // Top-left texture coordinate

    // Update the VBO with the new vertices data
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Update buttonPositions if it's a button
    if (type == BUTTON) {
        buttonPositions.clear();
        Button buttonPosition;
        buttonPosition.topLeft = glm::vec2(x, y + height);
        buttonPosition.topRight = glm::vec2(x + width, y + height);
        buttonPosition.bottomRight = glm::vec2(x + width, y);
        buttonPosition.bottomLeft = glm::vec2(x, y);
        buttonPositions.push_back(buttonPosition);
    }
}


void RenderText(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VAO, unsigned int VBO)
{
    s.use();
    glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    std::string::const_iterator c;
    for(c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        if (!ch.TextureID) {
            std::cout << "Warning: Missing texture for character: " << *c << std::endl;
            continue;
        }

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos, ypos + h, 0.0f, 0.0f},
            { xpos, ypos, 0.0f, 1.0f},
            { xpos + w, ypos, 1.0f, 1.0f},
            { xpos, ypos + h, 0.0f, 0.0f},
            { xpos + w, ypos, 1.0f, 1.0f},
            { xpos + w, ypos + h, 1.0f, 0.0f}
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale; // Advance cursor for next glyph
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void createVaoVbo(GLuint &vaoID, GLuint &vboID, unsigned int sizeOfVertices, unsigned int stride, unsigned int offset, GLenum usageType)
{
    glGenVertexArrays(1, &vaoID);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "Error generating VAO: " << error << std::endl;
    }

    glGenBuffers(1, &vboID);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "Error generating VBO: " << error << std::endl;
    }

    // Set up VAO
    glBindVertexArray(vaoID);

    // Bind VBO and allocate space
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeOfVertices * sizeof(float), nullptr, usageType);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set texture attribute pointers
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

}


void initalizeTextRendering(FT_Library &ft, FT_Face &face)
{   
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-allignment restriction
    
    for(unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if(FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
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
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


unsigned int createButton()
{
    float vertices[] = 
    { 
        // positions        // texture coords
        20.0f,  550.0f, 0.0f,  0.0f, 1.0f, // Top-left
        80.0f, 550.0f, 0.0f,  1.0f, 1.0f, // Top-right
        80.0f,  500.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right

        20.0f,  550.0f, 0.0f,  0.0f, 1.0f, // Top-left
        80.0f,  500.0f, 0.0f,  1.0f, 1.0f, // Bottom-right
        20.0f,  500.0f, 0.0f,  0.0f, 0.0f // Bottom-left
    };

    Button buttonPosition;
    buttonPosition.topLeft = glm::vec2(vertices[0], vertices[1]);
    buttonPosition.topRight = glm::vec2(vertices[5], vertices[6]);
    buttonPosition.bottomRight = glm::vec2(vertices[10], vertices[11]);
    buttonPosition.bottomLeft = glm::vec2(vertices[25], vertices[26]);


    buttonPositions.push_back(buttonPosition);
    unsigned int VAO = createRectangle(vertices, sizeof(vertices));
    return VAO;
}


unsigned int createMenuQuad()
{
    float vertices[] = {
        // positions        // texture coords
        0.0f,  600.0f, 0.0f,  0.0f, 1.0f,
        100.0f, 600.0f, 0.0f,  1.0f, 1.0f,
        100.0f,  0.0f, 0.0f,  1.0f, 0.0f,

        0.0f,  600.0f, 0.0f,  0.0f, 1.0f,
        100.0f,  0.0f, 0.0f,  1.0f, 1.0f,
        0.0f,  0.0f, 0.0f,  0.0f, 0.0f
    };

    unsigned int VAO = createRectangle(vertices, sizeof(vertices));
    return VAO;

}

std::vector<unsigned int>  createTextQuad()
{
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return std::vector<unsigned int>(VAO, VBO);

}

unsigned int createRectangle(float vertices[], unsigned int sizeOfVertices)
{
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeOfVertices, vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return VAO;
}

unsigned int createCube()
{
    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,  GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,  5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return VAO;
}

unsigned int generateTexture(const char* texturePath, Shader &ourShader)
{
    // load and create the texture
    unsigned int texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping parameters
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // set the texture filtering parameters
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip image on its y axis

    unsigned char *data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "FAILED TO LOAD TEXTURE" << std::endl;

    }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    
    return texture;
}

std::string GetCursorPosAsString()
{
    std::string cursorPos = "Cursor X Pos = " +  std::to_string((int)menuLastX) + ": Cursor Y Pos = " + std::to_string((int)menuLastY);
    return cursorPos;
}

std::string GetButtonPosAsString()
{
    std::string buttonPos = "Button Left X Pos = " + std::to_string((int)buttonPositions[0].bottomLeft.x) + ": Button Right X Pos = " + 
                            std::to_string((int)buttonPositions[0].bottomRight.x) + "\nButton Bottom Y Pos = " + std::to_string((int)buttonPositions[0].bottomLeft.y)
                            + ": Button Top Y Pos = " + std::to_string((int)buttonPositions[0].topLeft.y);   
    return buttonPos;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    const float cameraSpeed = 2.5f * delaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, delaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, delaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, delaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, delaTime);

    // Handle menu toggle
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !menuKeyPressed)
    {
        hasOpenedMenu = !hasOpenedMenu; // Togle menu state
        menuKeyPressed = true;
        
        if(hasOpenedMenu)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
    {
        menuKeyPressed = false; // Reset the key state when released
    }

    // Handle button press
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && inButton && hasOpenedMenu)
    {
        if(!buttonPressed)
        {
            isNegative = !isNegative;
            buttonPressed = true;
        }
    }
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    {
        buttonPressed = false;
    }

    // Developer commands
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && !xPressed)
    {
        coordinatesOn = !coordinatesOn;
        xPressed = true;
    }
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE)
    {
        xPressed = false;
    }
}
