#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "bb_math.h"

#include <SDL3/SDL.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

typedef GLuint gl_object;

static const u32 grass_block_pixels[8][8] = {
    { 0xFF5AC079, 0xFF4FAF6F, 0xFF5AC079, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F },
    { 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079 },
    { 0xFF5AC079, 0xFF5AC079, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F },
    { 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF5AC079 },
    { 0xFF5AC079, 0xFF429A5E, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF429A5E },
    { 0xFF4FAF6F, 0xFF5AC079, 0xFF67CB8A, 0xFF5AC079, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079 },
    { 0xFF5AC079, 0xFF67CB8A, 0xFF4FAF6F, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF4FAF6F },
    { 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF4FAF6F, 0xFF5AC079, 0xFF67CB8A },
};

static SDL_Window* window = NULL;
static u32 window_width = 0;
static u32 window_height = 0;

static bool was_quit = false;
static bool window_was_resized = false;

enum bb_button {
    bb_button_up,
    bb_button_down,
    bb_button_left,
    bb_button_right,
    bb_button_forward,
    bb_button_back,
    bb_button_count
};

static bool button_pressed[bb_button_count];
static bool button_released[bb_button_count];
static bool button_held[bb_button_count];

static f32 delta_time = 0.0f;

void bb_input_update(void)
{
    window_was_resized = false;

    memset(button_pressed, 0, sizeof(button_pressed));
    memset(button_released, 0, sizeof(button_pressed));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            was_quit = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            window_was_resized = true;
            window_width = event.window.data1;
            window_height = event.window.data2;
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            enum bb_button button = bb_button_count;
            switch (event.key.key)
            {
            case SDLK_SPACE:
                button = bb_button_up;
                break;
            case SDLK_LSHIFT:
                button = bb_button_down;
                break;
            case SDLK_A:
                button = bb_button_left;
                break;
            case SDLK_D:
                button = bb_button_right;
                break;
            case SDLK_W:
                button = bb_button_forward;
                break;
            case SDLK_S:
                button = bb_button_back;
                break;
            }
            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                button_pressed[button] = true;
                button_held[button] = true;
            }
            else
            {
                button_released[button] = true;
                button_held[button] = false;
            }
        }
        }
    }
}

/**
 * Load a file from path into a buffer
 *
 * If the returned buffer in NULL, the file could not be loaded
 * If the returned buffer is not NULL, it must be freed
 */
static char* bb_file_load(const char* path)
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
static usize bb_string_find_sub(const char* str, const char* sub)
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
static gl_object bb_shader_create(const char* shader_file)
{
    char* shader_source = bb_file_load(shader_file);
    if (shader_source == NULL)
    {
        return 0;
    }

    usize vertex_shader_pos = bb_string_find_sub(shader_source, "begin_vertex_shader");
    if (vertex_shader_pos == (usize)-1)
    {
        printf("Could not find \"begin_vertex_shader\" in %s\n", shader_file);
        return 0;
    }
    vertex_shader_pos += sizeof("begin_vertex_shader");

    usize vertex_shader_end = bb_string_find_sub(shader_source, "end_vertex_shader");
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

    usize fragment_shader_pos = bb_string_find_sub(shader_source, "begin_fragment_shader");
    if (fragment_shader_pos == (usize)-1)
    {
        printf("Could not find \"begin_fragment_shader\" in %s\n", shader_file);
        return 0;
    }
    fragment_shader_pos += sizeof("begin_fragment_shader");

    usize fragment_shader_end = bb_string_find_sub(shader_source, "end_fragment_shader");
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

    window = SDL_CreateWindow("Blocky Blocks", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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

    gl_object shader_program = bb_shader_create("shaders/basic.glsl");

    mat4 proj = bb_mat4_identity();
    mat4 view = bb_mat4_identity();
    mat4 model = bb_mat4_identity();

    gl_object vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    struct vertex {
        vec3 pos;
        vec2 uv;
    };

    struct vertex vertices[] = {
        {{-0.5f,  0.5f, -1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -1.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -1.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -1.0f}, {0.0f, 1.0f}},
    }; 

    gl_object vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void*)offsetof(struct vertex, uv));
    glEnableVertexAttribArray(1);

    u32 indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    gl_object index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    gl_object grass_block_texture = 0;
    glGenTextures(1, &grass_block_texture);
    glBindTexture(GL_TEXTURE_2D, grass_block_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, grass_block_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    u64 tick_prev = SDL_GetPerformanceCounter();
    const u64 tick_frequency = SDL_GetPerformanceFrequency();
    while (!was_quit)
    {
        u64 tick_current = SDL_GetPerformanceCounter();
        delta_time = (f32)(tick_current - tick_prev) / (f32)tick_frequency;
        tick_prev = tick_current;

        bb_input_update();
        if (window_was_resized)
        {
            glViewport(0, 0, window_width, window_height);
            proj = bb_mat4_perspective((f32)window_width / (f32)window_height, 3.14f / 2.0f, 0.1f, 100.0f);
        }

        if (button_held[bb_button_up])
            view.w.y -= 1.0f * delta_time;
        if (button_held[bb_button_down])
            view.w.y += 1.0f * delta_time;
        if (button_held[bb_button_right])
            view.w.x -= 1.0f * delta_time;
        if (button_held[bb_button_left])
            view.w.x += 1.0f * delta_time;
        if (button_held[bb_button_back])
            view.w.z -= 1.0f * delta_time;
        if (button_held[bb_button_forward])
            view.w.z += 1.0f * delta_time;

        glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);

        glUniformMatrix4fv(glGetUniformLocation(shader_program, "uProj"), 1, false, (f32*)&proj);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "uView"), 1, false, (f32*)&view);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "uModel"), 1, false, (f32*)&model);

        glUniform1i(glGetUniformLocation(shader_program, "uTex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass_block_texture);

        glBindVertexArray(vertex_array);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        SDL_GL_SwapWindow(window);
    }
}
