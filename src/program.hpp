#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glFunctionLoader.hpp"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

class Program
{
    GLFWwindow *window;
    unsigned int shaderProgram;
    unsigned int VBO, VAO, EBO;
    inline static int screenWidth = SCREEN_WIDTH;
    inline static int screenHeight = SCREEN_HEIGHT;
    unsigned int texture[5];

    static void frameBufferSizeCallback(GLFWwindow *window, int width, int height)
    {
        screenWidth = width;
        screenHeight = height;
        glViewport(0, 0, width, height);
    }

public:
    Program(void)
    {
        if (!glfwInit())
            exit(-1);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World!!", NULL, NULL);

        if (!window)
        {
            glfwTerminate();
            exit(-1);
        }

        loadAssets();
    }

    void runMailLoop(void)
    {
        float vertices[] = {
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
        };
        unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
        int frame = 0;
        GLint frameUniformLocation = glGetUniformLocation(shaderProgram, "iFrame");
        GLint mouseUniformLocation = glGetUniformLocation(shaderProgram, "iMouse");
        GLint timeUniformLocation = glGetUniformLocation(shaderProgram, "iTime");
        GLint uniformScreenSizeLocation = glGetUniformLocation(shaderProgram, "iResolution");
        double xPos, yPos;

        bool firstRun = true;

        while (!glfwWindowShouldClose(window))
        {

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, true);
            }

            frame++;

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture[1]);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, texture[2]);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, texture[3]);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, texture[4]);
            glUniform3f(uniformScreenSizeLocation, Program::screenWidth, Program::screenHeight, 1.0);

            glUniform1f(timeUniformLocation, glfwGetTime());

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) || firstRun)
            {
                firstRun = false;
                glfwGetCursorPos(window, &xPos, &yPos);
                int width, height;
                glfwGetWindowSize(window, &width, &height);
                yPos = height - yPos;
            }
            glUniform1i(frameUniformLocation, frame);

            glUniform4f(mouseUniformLocation, xPos, yPos, 0, 0);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    ~Program()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);
        glfwTerminate();
    }

    void loadAssets()
    {

        glfwMakeContextCurrent(window);
        loadOpenGLFunctions();
        glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

        // build and compile our shader program
        // ------------------------------------
        shaderProgram = shaderProgramCreateFromFilesShadertoy("assets/shader.vs", "assets/shader.fs");
        glUseProgram(shaderProgram);

        loadTexture("assets/channel0.png", "iChannel0", 0, &texture[0]);
        loadTexture("assets/channel1.png", "iChannel1", 1, &texture[1]);
        loadTexture("assets/channel3.png", "iChannel2", 2, &texture[2]);
        loadTexture("assets/channel2.png", "iChannel3", 3, &texture[3]);

        std::vector<std::string> faces = {
            "assets/cubemap/right.jpg",
            "assets/cubemap/left.jpg",
            "assets/cubemap/top.jpg",
            "assets/cubemap/bottom.jpg",
            "assets/cubemap/front.jpg",
            "assets/cubemap/back.jpg"};

        texture[4] = loadCubemap(faces);
    }

    unsigned int loadCubemap(std::vector<std::string> faces)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    void loadTexture(const char *fileName, const char *channel, int textureLocation, unsigned int *textId)
    {
        glGenTextures(1, textId);
        glBindTexture(GL_TEXTURE_2D, *textId);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int width, height, nrChannels;
        unsigned char *data = stbi_load(fileName, &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
            exit(-1);
        }
        stbi_image_free(data);
        glUniform1i(glGetUniformLocation(shaderProgram, channel), textureLocation);
    }
};