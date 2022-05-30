#pragma once
#include <GLFW/glfw3.h>
#include <GL/gl.h>

GLuint (*glCreateShader)(GLenum shaderType);
void (*glShaderSource)(GLuint shader,
                       GLsizei count,
                       const GLchar **string,
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

/* ############################################## */
void LoadOpenGLFunctions(void)
{
    {
        auto p = &glClear;
        auto fp = (void (*)(GLbitfield))glfwGetProcAddress("glClear");
        p = fp;
    }
    {
        auto p = &glClearColor;
        auto fp = (void (*)(GLclampf r, GLclampf g, GLclampf b, GLclampf a))glfwGetProcAddress("glClearColor");
        p = fp;
    }
    glCreateShader = (GLuint(*)(GLenum shaderType))glfwGetProcAddress("glCreateShader");
    glShaderSource = (void (*)(GLuint shader,
                               GLsizei count,
                               const GLchar **string,
                               const GLint *length))glfwGetProcAddress("glShaderSource");
    glCompileShader = (void (*)(GLuint shader))glfwGetProcAddress("glCompileShader");
    glGetShaderiv = (void (*)(GLuint shader,
                              GLenum pname,
                              GLint * params)) glfwGetProcAddress("glGetShaderiv");

    glGetShaderInfoLog = (void (*)(GLuint shader,
                                   GLsizei maxLength,
                                   GLsizei * length,
                                   GLchar * infoLog)) glfwGetProcAddress("glGetShaderInfoLog");

    glGetProgramInfoLog = (void (*)(GLuint program,
                                    GLsizei maxLength,
                                    GLsizei * length,
                                    GLchar * infoLog)) glfwGetProcAddress("glGetProgramInfoLog");

    glGetProgramiv = (void (*)(GLuint program,
                               GLenum pname,
                               GLint * params)) glfwGetProcAddress("glGetProgramiv");

    glCreateProgram = (GLuint(*)(void))glfwGetProcAddress("glCreateProgram");

    glAttachShader = (void (*)(GLuint program,
                               GLuint shader))glfwGetProcAddress("glAttachShader");

    glLinkProgram = (void (*)(GLuint program))glfwGetProcAddress("glLinkProgram");
    glDeleteShader = (void (*)(GLuint shader))glfwGetProcAddress("glDeleteShader");
}