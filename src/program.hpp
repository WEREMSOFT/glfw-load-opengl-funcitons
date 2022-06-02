#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glFunctionLoader.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

class Program
{
    GLFWwindow *window;
    unsigned int shaderProgram;
    unsigned int VBO, VAO, EBO;
    inline static int screenWidth = SCREEN_WIDTH;
    inline static int screenHeight = SCREEN_HEIGHT;

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

        glfwMakeContextCurrent(window);
        loadOpenGLFunctions();
        glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

        // build and compile our shader program
        // ------------------------------------
        shaderProgram = shaderProgramCreateFromFilesShadertoy("assets/shader.vs", "assets/shader.fs");
        // load textures
        {
            unsigned int texture1;
            // texture 1
            // ---------
            glGenTextures(1, &texture1);
            glBindTexture(GL_TEXTURE_2D, texture1);
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // set texture filtering parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            int width, height, nrChannels;
            unsigned char *data = stbi_load("assets/wall.jpg", &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
                exit(-1);
            }
            stbi_image_free(data);

            glGenTextures(1, &texture1);
            glBindTexture(GL_TEXTURE_2D, texture1);

            glUseProgram(shaderProgram);
            glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        }
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

        // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);
        int frame = 0;
        glUseProgram(shaderProgram);

        while (!glfwWindowShouldClose(window))
        {

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, true);
            }

            frame++;

            GLint uniformScreenSizeLocation = glGetUniformLocation(shaderProgram, "iResolution");
            glUniform3f(uniformScreenSizeLocation, Program::screenWidth, Program::screenHeight, 1.0);

            GLint timeUniformLocation = glGetUniformLocation(shaderProgram, "iTime");
            glUniform1f(timeUniformLocation, glfwGetTime());

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            GLint frameUniformLocation = glGetUniformLocation(shaderProgram, "iFrame");
            glUniform1i(frameUniformLocation, frame);

            GLint mouseUniformLocation = glGetUniformLocation(shaderProgram, "iMouse");
            glUniform4f(mouseUniformLocation, xpos, ypos, 0, 0);

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
};