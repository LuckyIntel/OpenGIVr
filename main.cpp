#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stbi/stb_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>
#define DEFAULT_ZOOM 1.0f
#define DEFAULT_DIMENSION 500
#define MAX_ZOOM 50.0f
#define MIN_ZOOM 0.2f
#define APP_AND_VERSION "OpenGIVr V1.0"
#define AUTHOR "LuckyIntel"
#define YEAR 2024

unsigned int WIDTH = 600;
unsigned int HEIGHT = 600;

unsigned int currentIMGWidth = DEFAULT_DIMENSION;
unsigned int currentIMGHeight = DEFAULT_DIMENSION;
std::string currentIMGColorChannel;

float aspectRatio = 1.0f;

char imagePath[1024];

bool showOpenImage = false;
bool showSettings = false;
bool showInfo = false;

glm::vec3 eyePos(0.0f, 0.0f, -1.0f);
glm::vec2 imgZoom(DEFAULT_ZOOM);

GLfloat vertices[] = {
    1.0f,  1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f,  1.0f, 0.0f, 1.0f
};

GLuint indices[] = {
    0, 1, 3,
    1, 2, 3 
};

char* filetobuf(const char *file)
{
    // This (filetobuf) code comes from KHRONOS GROUP tutorials!
    FILE *fptr;
    long length;
    char *buf;

    fptr = fopen(file, "rb"); /* Open file for reading */
    if (!fptr) /* Return NULL on failure */
        return NULL;
    fseek(fptr, 0, SEEK_END); /* Seek to the end of the file */
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length+1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */

    return buf; /* Return the buffer */
}

void onResize(GLFWwindow* window, int width, int height);
void onScroll(GLFWwindow* window, double xWheel, double yWheel);
void onKeyInput(GLFWwindow* window);

glm::vec2 makeSmallVEC2D(unsigned int Number1, unsigned int Number2)
{
    unsigned int Number = (Number1 > Number2) ? Number1 : Number2;
    unsigned int mockNumber = Number;

    double inDig = 0;

    while (mockNumber > 0)
    {
        mockNumber = floor(mockNumber / 10);
        inDig += 1.0;
    };

    inDig -= 0.5;

    return glm::vec2(Number1 / pow(10, inDig), Number2 / pow(10, inDig));
};

GLuint loadImageFromPath(const char imagePath[1024])
{

    GLuint id;
    GLint iW, iH, iCc, iF;
    unsigned char* img = stbi_load(imagePath, &iW, &iH, &iCc, 0);

    currentIMGWidth = iW;
    currentIMGHeight = iH;

    switch (iCc)
    {
        case 1: iF = GL_RED; currentIMGColorChannel = "Red(R)"; break;
        case 3: iF = GL_RGB; currentIMGColorChannel = "Red Green Blue(RGB)"; break;
        case 4: iF = GL_RGBA; currentIMGColorChannel = "Red Green Blue Alpha(RGBA)"; break;
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, iF, iW, iH, 0, iF, GL_UNSIGNED_BYTE, img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(img);
    return id;
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Making this false will not make you happy.

    stbi_set_flip_vertically_on_load(1);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGIVr(Empty)", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 300, GLFW_DONT_CARE, GLFW_DONT_CARE); // Limits for best use.
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSetScrollCallback(window, onScroll);
    glfwSwapInterval(1); // Vertical sync because you DON'T exactly need more than 60 FPS in an image viewer.
    
    gladLoadGL();

    GLchar* vSrc = filetobuf("shaders/default.vert");
    GLchar* fSrc = filetobuf("shaders/default.frag");
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertShader, 1, const_cast<const GLchar**>(&vSrc), nullptr);
    glShaderSource(fragShader, 1, const_cast<const GLchar**>(&fSrc), nullptr);
    glCompileShader(vertShader);
    glCompileShader(fragShader);

    GLuint sProg = glCreateProgram();
    glAttachShader(sProg, vertShader);
    glAttachShader(sProg, fragShader);
    glLinkProgram(sProg);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    GLuint myImg = 0;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    ImGuiIO io = ImGui::GetIO(); (void)io;

    glViewport(0, 0, WIDTH, HEIGHT);

    aspectRatio = (float)WIDTH / HEIGHT;

    glm::mat4 model(1.0f);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Transparency
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open An Existing Image")) showOpenImage = !showOpenImage;
                if (ImGui::MenuItem("Settings")) showSettings = !showSettings;
                if (ImGui::MenuItem("Close")) glfwSetWindowShouldClose(window, GLFW_TRUE);
                ImGui::EndMenu();
            };
            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Get Image Info")) showInfo = !showInfo;
                if (ImGui::MenuItem("Reset Zoom"))
                {
                    imgZoom = glm::vec2(DEFAULT_ZOOM);
                    eyePos.x = eyePos.y = 0.0f;
                };
                ImGui::EndMenu();
            };
            ImGui::EndMainMenuBar();
        };

        if (showOpenImage)
        {
            ImGui::SetNextWindowSize(ImVec2(350.0f, 120.0f));
            ImGui::Begin("Open Image", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::Text("Image Path : ");
            ImGui::InputText("Enter the image path...", imagePath, 1024);
            if (ImGui::Button("Open Image"))
            {
                //std::cout << imagePath << "\n";
                myImg = loadImageFromPath(imagePath);
                glfwSetWindowTitle(window, "OpenGIVr(Viewing An Image)");
                showOpenImage = false;
            };
            ImGui::End();
        };

        if (showSettings)
        {
            ImGui::SetNextWindowSize(ImVec2(250.0f, 120.0f));
            ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::Text("%s", APP_AND_VERSION);
            ImGui::Text("Made by %s in %i", AUTHOR, YEAR);
            ImGui::End();
        };

        if (showInfo)
        {
            ImGui::SetNextWindowSize(ImVec2(300.0f, 120.0f));
            ImGui::Begin("Image Info", nullptr, ImGuiWindowFlags_NoResize);
            ImGui::Text("Resolution : %ix%i", currentIMGWidth, currentIMGHeight);
            ImGui::Text("Color Channel : %s", currentIMGColorChannel.c_str());
            ImGui::Text("Zoom : %.2fx", (float)imgZoom.x);
            ImGui::End();
        };

        if (showOpenImage != true) onKeyInput(window);

        glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f / aspectRatio, 1.0f / aspectRatio, 0.1f, 20.0f);
        //glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), eyePos);

        bool renderImage = (myImg != 0);

        if (renderImage) glBindTexture(GL_TEXTURE_2D, myImg);
        glUseProgram(sProg);
        glUniformMatrix4fv(glGetUniformLocation(sProg, "pvm"), 1, GL_FALSE, glm::value_ptr(projection * view * model));
        glUniform2fv(glGetUniformLocation(sProg, "size"), 1, glm::value_ptr(makeSmallVEC2D(currentIMGWidth, currentIMGHeight)));
        glUniform2fv(glGetUniformLocation(sProg, "zoomX"), 1, glm::value_ptr(imgZoom));
        glUniform1i(glGetUniformLocation(sProg, "applyTexture"), (bool)renderImage);
        if (renderImage) glUniform1i(glGetUniformLocation(sProg, "photoTexture"), 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    };

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glDeleteProgram(sProg);
    glfwDestroyWindow(window);
    glfwTerminate();
};

void onResize(GLFWwindow* window, int width, int height)
{ 
    WIDTH = width; HEIGHT = height;
    if (WIDTH == 0 && HEIGHT == 0) aspectRatio = 1.0f;
    else aspectRatio = (float)WIDTH / HEIGHT;
    glViewport(0, 0, WIDTH, HEIGHT);
};

void onScroll(GLFWwindow* window, double xWheel, double yWheel)
{
    float resizeSpeed = 0.1f;
    if (yWheel > 0 && glm::all(glm::lessThan(imgZoom, glm::vec2(MAX_ZOOM)))) imgZoom +=  glm::vec2(1.0f) * resizeSpeed;
    else if (yWheel < 0 && glm::all(glm::lessThan(glm::vec2(MIN_ZOOM), imgZoom))) imgZoom += glm::vec2(1.0f) * -resizeSpeed;  
};

void onKeyInput(GLFWwindow* window)
{
    float moveSpeed = 0.1f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) eyePos.y -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) eyePos.x += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) eyePos.y += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) eyePos.x -= moveSpeed;
};