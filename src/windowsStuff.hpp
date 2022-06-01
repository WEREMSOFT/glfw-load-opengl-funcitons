#pragma once

// disgusting windows stuff

#if defined(_WIN32) || defined(_WIN64)

#ifndef GLchar
typedef char GLchar;
typedef size_t GLsizeiptr;
#define GL_COMPILE_STATUS 0x8B81
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_LINK_STATUS 0x8B82
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#endif

#endif