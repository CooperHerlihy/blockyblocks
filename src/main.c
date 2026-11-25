#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

typedef GLuint gl_object;

/**
 * Load a file from path into a buffer
 *
 * If the returned buffer in NULL, the file could not be loaded
 * If the returned buffer is not NULL, it must be freed
 */
char* file_load(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("fopen failed to open %s\n", path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        printf("fseek failed for %s\n", path);
        fclose(file);
        return NULL;
    }

    long size = ftell(file);
    if (size < 0)
    {
        printf("ftell failed for %s\n", path);
        fclose(file);
        return NULL;
    }
    rewind(file);

    char* buffer = malloc((usize)size + 1);
    if (buffer == NULL)
    {
        printf("malloc failed for %s\n", path);
        fclose(file);
        return NULL;
    }

    usize read = fread(buffer, 1, (usize)size, file);
    fclose(file);
    if (read != (usize)size)
    {
        printf("fread failed for %s\n", path);
        free(buffer);
        return NULL;
    }

    buffer[read] = '\0';
    return buffer;
}

/**
 * Load the shader files and create a shader program
 *
 * If the returned object is 0, the shaders could not be loaded
 */
gl_object shader_create(const char* vertex_shader_file, const char* fragment_shader_file)
{
    char* vertex_shader_source = file_load(vertex_shader_file);
    if (vertex_shader_source == NULL)
    {
        return 0;
    }

    char* fragment_shader_source = file_load(fragment_shader_file);
    if (fragment_shader_source == NULL)
    {
        free(vertex_shader_source);
        return 0;
    }

    gl_object vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, (const char* const*)&vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    int vertex_shader_success = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_success);
    if (!vertex_shader_success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        printf("glCompileShader failed to compile vertex shader: %s", log);
    }

    gl_object fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, (const char* const*)&fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    int fragment_shader_success = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_shader_success);
    if (!fragment_shader_success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        printf("glCompileShader failed to compile fragment shader: %s", log);
    }

    gl_object shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    int link_shader_success = 0;
    glGetShaderiv(shader_program, GL_LINK_STATUS, &link_shader_success);
    if (!link_shader_success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(shader_program, 512, NULL, log);
        printf("glCompileShader failed to link shaders: %s", log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    free(vertex_shader_source);
    free(fragment_shader_source);
    return shader_program;
}

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Blocky Blocks", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == NULL)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress))
    {
        printf("GLADloadfunc failed: %s\n", SDL_GetError());
        return 1;
    }

    float vertices[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    }; 

    gl_object vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    u32 indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    gl_object index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    gl_object shader_program = shader_create("shaders/basic.vert", "shaders/basic.frag");

    gl_object vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float[2]), NULL);
    glEnableVertexAttribArray(0);

    for (;;)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                goto quit;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                glViewport(0, 0, event.window.data1, event.window.data2);
                break;
            }
        }

        glClearColor(0.1f, 0.3, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vertex_array);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        SDL_GL_SwapWindow(window);
    }
quit:
    return 0;
}
