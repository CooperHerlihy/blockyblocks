#ifndef BB_H
#define BB_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t usize;

#include <SDL3/SDL.h>
#include <glad/gl.h>

typedef GLuint gl_object;

static inline const char* bb_gl_error_to_string(GLenum e)
{
    switch (e)
    {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            return "unknown gl error enum";
    }
}

static inline void bb_gl_error(const char* file, int line)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        printf("OpenGl error in file %s:%d: %s\n", file, line, bb_gl_error_to_string(e));
    }
}

#define BB_GET_GL_ERROR() do { bb_gl_error(__FILE__, __LINE__); } while(0)

#endif // BB_H
