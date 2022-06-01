#pragma once

#include <GL/gl.h>
#include <string.h>
#include "windowsStuff.hpp"

void (*glDeleteProgram)(GLuint program);
void (*glDeleteBuffers)(GLsizei n,
                        const GLuint *buffers);
void (*glGenBuffers)(GLsizei n, GLuint *buffers);
GLuint (*glCreateShader)(GLenum shaderType);
void (*glShaderSource)(GLuint shader,
                       GLsizei count,
                       const GLchar **str,
                       const GLint *length);
void (*glCompileShader)(GLuint shader);
void (*glGetShaderiv)(GLuint shader,
                      GLenum pname,
                      GLint *params);
void (*glGetShaderInfoLog)(GLuint shader,
                           GLsizei maxLength,
                           GLsizei *length,
                           GLchar *infoLog);
void (*glGetProgramInfoLog)(GLuint program,
                            GLsizei maxLength,
                            GLsizei *length,
                            GLchar *infoLog);
void (*glGetProgramiv)(GLuint program,
                       GLenum pname,
                       GLint *params);
GLuint (*glCreateProgram)(void);
void (*glAttachShader)(GLuint program,
                       GLuint shader);
void (*glLinkProgram)(GLuint program);
void (*glDeleteShader)(GLuint shader);
void (*glGenVertexArrays)(GLsizei n,
                          GLuint *arrays);
void (*GenBuffers)(GLsizei n,
                   GLuint *buffers);
void (*glBindVertexArray)(GLuint array);
void (*glBindBuffer)(GLenum target,
                     GLuint buffer);

void (*glBufferData)(GLenum target,
                     GLsizeiptr size,
                     const void *data,
                     GLenum usage);

void (*glVertexAttribPointer)(GLuint index,
                              GLint size,
                              GLenum type,
                              GLboolean normalized,
                              GLsizei stride,
                              const void *pointer);

void (*glEnableVertexAttribArray)(GLuint index);

void (*glUseProgram)(GLuint program);

void (*glDeleteVertexArrays)(GLsizei n,
                             const GLuint *arrays);

void (*DeleteBuffers)(GLsizei n,
                      const GLuint *buffers);

void (*DeleteProgram)(GLuint program);

GLint (*glGetUniformLocation)(GLuint program, const GLchar *uniformName);

void (*glUniform1f)(GLuint id, GLfloat x);
void (*glUniform2f)(GLuint id, GLfloat x, GLfloat y);
void (*glUniform3f)(GLuint id, GLfloat x, GLfloat y, GLfloat z);
void (*glUniform4f)(GLuint id, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void (*glUniform2i)(GLuint id, GLuint x, GLuint y);
void (*glUniform1i)(GLuint id, GLint x);

// void (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

#define bindFunctionRef(a, b)                \
    {                                        \
        auto p = a;                          \
        auto fp = (b)glfwGetProcAddress(#a); \
        p = fp;                              \
    }

#define bindFunction(a, b) a = (b)glfwGetProcAddress(#a);

/* ############################################## */

void loadOpenGLFunctions(void)
{
    bindFunctionRef(glViewport, void (*)(GLint x, GLint y, GLsizei width, GLsizei height));
    bindFunction(glUniform1i, void (*)(GLuint id, GLint x));
    bindFunction(glUniform1f, void (*)(GLuint id, GLfloat x));
    bindFunction(glUniform2f, void (*)(GLuint id, GLfloat x, GLfloat y));
    bindFunction(glUniform3f, void (*)(GLuint id, GLfloat x, GLfloat y, GLfloat z));
    bindFunction(glUniform4f, void (*)(GLuint id, GLfloat x, GLfloat y, GLfloat z, GLfloat w));
    bindFunction(glUniform2i, void (*)(GLuint id, GLuint x, GLuint y));
    bindFunction(glGetUniformLocation, GLint(*)(GLuint program, const GLchar *uniformName));
    bindFunctionRef(glClear, void (*)(GLbitfield));
    bindFunctionRef(glClearColor, void (*)(GLclampf r, GLclampf g, GLclampf b, GLclampf a));
    bindFunction(glCreateShader, GLuint(*)(GLenum shaderType));
    bindFunction(glShaderSource, void (*)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length));
    bindFunction(glCompileShader, void (*)(GLuint shader));
    bindFunction(glGetShaderiv, void (*)(GLuint shader,
                                         GLenum pname,
                                         GLint * params));
    bindFunction(glGetShaderInfoLog, void (*)(GLuint shader,
                                              GLsizei maxLength,
                                              GLsizei * length,
                                              GLchar * infoLog));
    bindFunction(glGetProgramInfoLog, void (*)(GLuint program,
                                               GLsizei maxLength,
                                               GLsizei * length,
                                               GLchar * infoLog));
    bindFunction(glGetProgramiv, void (*)(GLuint program,
                                          GLenum pname,
                                          GLint * params));
    bindFunction(glCreateProgram, GLuint(*)(void));
    bindFunction(glAttachShader, void (*)(GLuint program,
                                          GLuint shader));
    bindFunction(glLinkProgram, void (*)(GLuint program));
    bindFunction(glDeleteShader, void (*)(GLuint shader));
    bindFunction(glGenVertexArrays, void (*)(GLsizei n, GLuint * arrays));
    bindFunction(glGenBuffers, void (*)(GLsizei n, GLuint * buffers));
    bindFunction(glBindVertexArray, void (*)(GLuint array));
    bindFunction(glBindBuffer, void (*)(GLenum target, GLuint buffer));
    bindFunction(glBufferData, void (*)(GLenum target,
                                        GLsizeiptr size,
                                        const void *data,
                                        GLenum usage));
    bindFunction(glVertexAttribPointer, void (*)(GLuint index,
                                                 GLint size,
                                                 GLenum type,
                                                 GLboolean normalized,
                                                 GLsizei stride,
                                                 const void *pointer));
    bindFunction(glEnableVertexAttribArray, void (*)(GLuint index));
    bindFunction(glUseProgram, void (*)(GLuint program));
    bindFunction(glDeleteVertexArrays, void (*)(GLsizei n, const GLuint *arrays));
    bindFunction(glDeleteBuffers, void (*)(GLsizei n, const GLuint *buffers));
    bindFunction(glDeleteProgram, void (*)(GLuint program));
}

static int shaderCreateFromFile(const char *fileName, unsigned int *vertexShader, int shaderType)
{
    int returnValue = 0;
    long fileSize = 0;
    char *shaderCode = NULL;
    *vertexShader = glCreateShader(shaderType);

    FILE *fp = fopen(fileName, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        returnValue = -1;
        goto error_handler;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        perror("Error obtaining the file size ");
        returnValue = -1;
        goto error_handler;
    }

    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    shaderCode = (char *)calloc(fileSize + 1, sizeof(char));

    if (shaderCode == NULL)
    {
        returnValue = -1;
        goto error_handler;
    }

    if (fread(shaderCode, sizeof(char), fileSize, fp) == 0)
    {
        printf("Error reading file: %s\n", fileName);
        exit(-1);
    }

    glShaderSource(*vertexShader, 1, (const char **)&shaderCode, NULL);

    glCompileShader(*vertexShader);

    int shaderCompilationSuccess;
    char infoLog[512];

    glGetShaderiv(*vertexShader, GL_COMPILE_STATUS, &shaderCompilationSuccess);

    if (!shaderCompilationSuccess)
    {
        glGetShaderInfoLog(*vertexShader, 512, NULL, infoLog);
        printf("%s::%s : vertex shader compilation failed. Error %s\n", __FILE__, __func__, infoLog);
        returnValue = -1;
        goto error_handler;
    }

error_handler:
    if (shaderCode != NULL)
        free(shaderCode);
    if (fp != NULL)
        fclose(fp);

    return returnValue;
}

unsigned int shaderProgramCreateFromFiles(const char *vertexShaderPath, const char *fragmentShaderPath)
{
    unsigned int vs, fs;

    shaderCreateFromFile(vertexShaderPath, &vs, GL_VERTEX_SHADER);
    shaderCreateFromFile(fragmentShaderPath, &fs, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    if (vs == 0 || fs == 0)
    {
        fprintf(stderr, "error loading shadfer files\n");
        exit(-1);
    }
    glLinkProgram(shaderProgram);

    int success;

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("%s::%s - Error linking shader program: %s\n", __FILE__, __func__, infoLog);
        exit(-1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return shaderProgram;
}

static int createShadertoyFSFromFile(const char *fileName, unsigned int *vertexShader, int shaderType)
{
    std::string shaderHeader = "#version 330 core\n"
                               "out vec4 FragColor;\n"
                               "in vec2 TexCoord;\n"
                               "uniform vec3 iResolution;\n"
                               "uniform float iTime;\n"
                               "uniform vec4 iMouse;\n"
                               "uniform int iFrame;\n";

    std::string shaderFooter = "void main(){\n"
                               "mainImage(FragColor, TexCoord * iResolution.xy);\n"
                               "}\n";
    int returnValue = 0;
    long fileSize = 0;
    char *shaderCode = NULL;
    *vertexShader = glCreateShader(shaderType);

    FILE *fp = fopen(fileName, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        exit(-1);
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        perror("Error obtaining the file size ");
        exit(-1);
    }

    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    shaderCode = (char *)calloc(fileSize + 1, sizeof(char));

    if (shaderCode == NULL)
    {
        exit(-1);
    }

    if (fread(shaderCode, sizeof(char), fileSize, fp) == 0)
    {
        printf("Error reading file: %s\n", fileName);
        exit(-1);
    }

    std::string shaderCodeString(shaderCode);

    std::string finalShaderCodeString = shaderHeader + shaderCodeString + shaderFooter;

    char *shaderCoderCString = (char *)(finalShaderCodeString.c_str());

    glShaderSource(*vertexShader, 1, (const char **)&shaderCoderCString, NULL);

    glCompileShader(*vertexShader);

    int shaderCompilationSuccess;
    char infoLog[512];

    glGetShaderiv(*vertexShader, GL_COMPILE_STATUS, &shaderCompilationSuccess);

    if (!shaderCompilationSuccess)
    {
        glGetShaderInfoLog(*vertexShader, 512, NULL, infoLog);
        printf("%s::%s : vertex shader compilation failed. Error %s\n", __FILE__, __func__, infoLog);
        returnValue = -1;
        goto error_handler;
    }

error_handler:
    if (shaderCode != NULL)
        free(shaderCode);
    if (fp != NULL)
        fclose(fp);

    return returnValue;
}

unsigned int shaderProgramCreateFromFilesShadertoy(const char *vertexShaderPath, const char *fragmentShaderPath)
{
    unsigned int vs, fs;

    shaderCreateFromFile(vertexShaderPath, &vs, GL_VERTEX_SHADER);
    createShadertoyFSFromFile(fragmentShaderPath, &fs, GL_FRAGMENT_SHADER);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    if (vs == 0 || fs == 0)
    {
        fprintf(stderr, "error loading shadfer files\n");
        exit(-1);
    }
    glLinkProgram(shaderProgram);

    int success;

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("%s::%s - Error linking shader program: %s\n", __FILE__, __func__, infoLog);
        exit(-1);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return shaderProgram;
}
