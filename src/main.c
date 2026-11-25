#include <math.h>
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
static char* file_load(const char* path)
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
 * Search for a substring
 */
static usize string_find_sub(const char* str, const char* sub)
{
    if (sub == NULL)
        return 0;
    if (str == NULL)
        return (usize)-1;

    usize str_length = strlen(str);
    usize sub_length = strlen(sub);
    if (str_length < sub_length)
        return (usize)-1;

    usize pos = 0;
    while (memcmp(str + pos, sub, sub_length) != 0)
    {
        if ((str + pos)[sub_length] == '\0')
            return (usize)-1;
        ++pos;
    }

    return pos;
}

/**
 * Load the shader files and create a shader program
 *
 * If the returned object is 0, the shaders could not be loaded
 */
static gl_object shader_create(const char* shader_file)
{
    char* shader_source = file_load(shader_file);
    if (shader_source == NULL)
    {
        return 0;
    }

    usize vertex_shader_pos = string_find_sub(shader_source, "begin_vertex_shader");
    if (vertex_shader_pos == (usize)-1)
    {
        printf("Could not find \"begin_vertex_shader\" in %s\n", shader_file);
        return 0;
    }
    vertex_shader_pos += sizeof("begin_vertex_shader");

    usize vertex_shader_end = string_find_sub(shader_source, "end_vertex_shader");
    if (vertex_shader_end == (usize)-1)
    {
        printf("Could not find \"end_vertex_shader\" in %s\n", shader_file);
        return 0;
    }

    gl_object vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* vertex_shader_source = shader_source + vertex_shader_pos;
    const GLint vertex_shader_length = vertex_shader_end - vertex_shader_pos;
    glShaderSource(vertex_shader, 1, &vertex_shader_source, &vertex_shader_length);
    glCompileShader(vertex_shader);

    int vertex_shader_success = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_shader_success);
    if (!vertex_shader_success)
    {
        char log[512] = {0};
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        printf("glCompileShader failed to compile vertex shader: %s", log);
    }

    usize fragment_shader_pos = string_find_sub(shader_source, "begin_fragment_shader");
    if (fragment_shader_pos == (usize)-1)
    {
        printf("Could not find \"begin_fragment_shader\" in %s\n", shader_file);
        return 0;
    }
    fragment_shader_pos += sizeof("begin_fragment_shader");

    usize fragment_shader_end = string_find_sub(shader_source, "end_fragment_shader");
    if (fragment_shader_end == (usize)-1)
    {
        printf("Could not find \"end_fragment_shader\" in %s\n", shader_file);
        return 0;
    }

    gl_object fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* fragment_shader_source = shader_source + fragment_shader_pos;
    const GLint fragment_shader_length = fragment_shader_end - fragment_shader_pos;
    glShaderSource(fragment_shader, 1, &fragment_shader_source, &fragment_shader_length);
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
    free(shader_source);
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

    gl_object shader_program = shader_create("shaders/basic.glsl");

    gl_object vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float[2]), NULL);
    glEnableVertexAttribArray(0);

    bool left_pressed = false;
    bool right_pressed = false;
    bool up_pressed = false;
    bool down_pressed = false;
    f32 pos_x = 0.0f;
    f32 pos_y = 0.0f;

    u64 tick_prev = SDL_GetPerformanceCounter();
    const u64 tick_frequency = SDL_GetPerformanceFrequency();
    f32 current_time = 0.0f;
    for (;;)
    {
        u64 tick_current = SDL_GetPerformanceCounter();
        f32 delta_time = (f32)(tick_current - tick_prev) / (f32)tick_frequency;
        tick_prev = tick_current;
        current_time += delta_time;

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
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
            {
                bool down = event.type == SDL_EVENT_KEY_DOWN;
                switch (event.key.key)
                {
                case SDLK_W:
                    up_pressed = down;
                    break;
                case SDLK_A:
                    left_pressed = down;
                    break;
                case SDLK_S:
                    down_pressed = down;
                    break;
                case SDLK_D:
                    right_pressed = down;
                    break;
                }
                break;
            }
            }
        }

        glClearColor(0.1f, 0.3, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        if (left_pressed)
            pos_x -= 1.0f * delta_time;
        if (right_pressed)
            pos_x += 1.0f * delta_time;
        if (down_pressed)
            pos_y -= 1.0f * delta_time;
        if (up_pressed)
            pos_y += 1.0f * delta_time;
        glUniform2f(glGetUniformLocation(shader_program, "uPos"), pos_x, pos_y);

        f32 colorVal = sinf(current_time);
        colorVal *= colorVal;
        glUniform4f(glGetUniformLocation(shader_program, "uColor"), colorVal, 1.0f - colorVal, 0.2f, 1.0f);

        glBindVertexArray(vertex_array);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        SDL_GL_SwapWindow(window);
    }
quit:
    return 0;
}
