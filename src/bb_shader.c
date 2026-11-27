#include "bb_shader.h"

#include <stdio.h>
#include <stdlib.h>

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
gl_object bb_shader_create(const char* shader_file)
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
    const GLint vertex_shader_length = (GLint)vertex_shader_end - (GLint)vertex_shader_pos;
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
    const GLint fragment_shader_length = (GLint)fragment_shader_end - (GLint)fragment_shader_pos;
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
    glGetProgramiv(shader_program, GL_LINK_STATUS, &link_shader_success);
    if (!link_shader_success)
    {
        char log[512] = {0};
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        printf("glCompileShader failed to link shaders: %s", log);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    free(shader_source);
    return shader_program;
}

