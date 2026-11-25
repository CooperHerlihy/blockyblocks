#define GLAD_GL_IMPLEMENTATION
#include "bb.h"
#include "bb_math.h"
#include "bb_input.h"
#include "bb_shader.h"

#include <stdio.h>
#include <stddef.h>

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

static vec3 player_position = {0};
static quat player_rotation = {1.0f, 0.0f, 0.0f, 0.0f};

enum bb_block {
    bb_block_air = 0,
    bb_block_grass,
    bb_block_count,
};

#define bb_chunk_height 64
#define bb_chunk_width 16
#define bb_chunk_depth 16

struct bb_chunk {
    enum bb_block blocks[bb_chunk_height][bb_chunk_depth][bb_chunk_width];
};

struct bb_chunk_vertex {
    vec3 pos;
    vec2 uv;
};

struct bb_chunk_mesh {
    usize vertex_count;
    struct bb_chunk_vertex* vertices;
    usize index_count;
    u32* indices;

    gl_object vertex_array;
    gl_object vertex_buffer;
    gl_object index_buffer;
};

/**
 * Allocates a new mesh from the chunk
 *
 * Must be destroyed
 */
static struct bb_chunk_mesh bb_chunk_mesh_create(struct bb_chunk* chunk)
{
    usize vertex_capacity = 128;
    usize vertex_count = 0;
    struct bb_chunk_vertex* vertices = malloc(vertex_capacity * sizeof(*vertices));

    usize index_capacity = 128;
    usize index_count = 0;
    u32* indices = malloc(index_capacity * sizeof(*indices));

    for (usize y = 0; y < bb_chunk_height; ++y)
    {
        for (usize z = 0; z < bb_chunk_depth; ++z)
        {
            for (usize x = 0; x < bb_chunk_width; ++x)
            {
                if (chunk->blocks[y][z][x] != bb_block_air)
                {
                    f32 cx = (f32)x;
                    f32 cy = (f32)y;
                    f32 cz = (f32)z;

                    if (y == 0 || chunk->blocks[y - 1][z][x] == bb_block_air)
                    {
                        if (vertex_count + 4 > vertex_capacity)
                        {
                            vertex_capacity *= 2;
                            vertices = realloc(vertices, vertex_capacity * sizeof(*vertices));
                        }
                        if (index_count + 6 > index_capacity)
                        {
                            index_capacity *= 2;
                            indices = realloc(indices, index_capacity * sizeof(*indices));
                        }
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy - 0.5f, cz + 0.5f},
                            (vec2){0.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy - 0.5f, cz - 0.5f},
                            (vec2){1.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy - 0.5f, cz - 0.5f},
                            (vec2){1.0f, 1.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy - 0.5f, cz + 0.5f},
                            (vec2){0.0f, 1.0f},
                        };
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 3;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 1;
                    }
                    if (y == bb_chunk_height - 1 || chunk->blocks[y + 1][z][x] == bb_block_air)
                    {
                        if (vertex_count + 4 > vertex_capacity)
                        {
                            vertex_capacity *= 2;
                            vertices = realloc(vertices, vertex_capacity * sizeof(*vertices));
                        }
                        if (index_count + 6 > index_capacity)
                        {
                            index_capacity *= 2;
                            indices = realloc(indices, index_capacity * sizeof(*indices));
                        }
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy + 0.5f, cz - 0.5f},
                            (vec2){0.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy + 0.5f, cz + 0.5f},
                            (vec2){1.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy + 0.5f, cz + 0.5f},
                            (vec2){1.0f, 1.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy + 0.5f, cz - 0.5f},
                            (vec2){0.0f, 1.0f},
                        };
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 3;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 1;
                    }
                    if (z == 0 || chunk->blocks[y][z - 1][x] == bb_block_air)
                    {
                        if (vertex_count + 4 > vertex_capacity)
                        {
                            vertex_capacity *= 2;
                            vertices = realloc(vertices, vertex_capacity * sizeof(*vertices));
                        }
                        if (index_count + 6 > index_capacity)
                        {
                            index_capacity *= 2;
                            indices = realloc(indices, index_capacity * sizeof(*indices));
                        }
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy - 0.5f, cz - 0.5f},
                            (vec2){0.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy - 0.5f, cz - 0.5f},
                            (vec2){1.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy + 0.5f, cz - 0.5f},
                            (vec2){1.0f, 1.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy + 0.5f, cz - 0.5f},
                            (vec2){0.0f, 1.0f},
                        };
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 3;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 1;
                    }
                    if (z == bb_chunk_depth - 1 || chunk->blocks[y][z + 1][x] == bb_block_air)
                    {
                        if (vertex_count + 4 > vertex_capacity)
                        {
                            vertex_capacity *= 2;
                            vertices = realloc(vertices, vertex_capacity * sizeof(*vertices));
                        }
                        if (index_count + 6 > index_capacity)
                        {
                            index_capacity *= 2;
                            indices = realloc(indices, index_capacity * sizeof(*indices));
                        }
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy - 0.5f, cz + 0.5f},
                            (vec2){0.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy - 0.5f, cz + 0.5f},
                            (vec2){1.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy + 0.5f, cz + 0.5f},
                            (vec2){1.0f, 1.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy + 0.5f, cz + 0.5f},
                            (vec2){0.0f, 1.0f},
                        };
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 3;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 1;
                    }
                    if (x == 0 || chunk->blocks[y][z][x - 1] == bb_block_air)
                    {
                        if (vertex_count + 4 > vertex_capacity)
                        {
                            vertex_capacity *= 2;
                            vertices = realloc(vertices, vertex_capacity * sizeof(*vertices));
                        }
                        if (index_count + 6 > index_capacity)
                        {
                            index_capacity *= 2;
                            indices = realloc(indices, index_capacity * sizeof(*indices));
                        }
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy - 0.5f, cz - 0.5f},
                            (vec2){0.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy - 0.5f, cz + 0.5f},
                            (vec2){1.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy + 0.5f, cz + 0.5f},
                            (vec2){1.0f, 1.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx - 0.5f, cy + 0.5f, cz - 0.5f},
                            (vec2){0.0f, 1.0f},
                        };
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 3;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 1;
                    }
                    if (x == bb_chunk_width - 1 || chunk->blocks[y][z][x + 1] == bb_block_air)
                    {
                        if (vertex_count + 4 > vertex_capacity)
                        {
                            vertex_capacity *= 2;
                            vertices = realloc(vertices, vertex_capacity * sizeof(*vertices));
                        }
                        if (index_count + 6 > index_capacity)
                        {
                            index_capacity *= 2;
                            indices = realloc(indices, index_capacity * sizeof(*indices));
                        }
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy - 0.5f, cz + 0.5f},
                            (vec2){0.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy - 0.5f, cz - 0.5f},
                            (vec2){1.0f, 0.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy + 0.5f, cz - 0.5f},
                            (vec2){1.0f, 1.0f},
                        };
                        vertices[vertex_count++] = (struct bb_chunk_vertex){
                            (vec3){cx + 0.5f, cy + 0.5f, cz + 0.5f},
                            (vec2){0.0f, 1.0f},
                        };
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 3;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 4;
                        indices[index_count++] = vertex_count - 2;
                        indices[index_count++] = vertex_count - 1;
                    }
                }
            }
        }
    }

    gl_object vertex_array = 0;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    gl_object vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(*vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct bb_chunk_vertex), (void*)offsetof(struct bb_chunk_vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct bb_chunk_vertex), (void*)offsetof(struct bb_chunk_vertex, uv));
    glEnableVertexAttribArray(1);

    gl_object index_buffer = 0;
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(*indices), indices, GL_STATIC_DRAW);

    return (struct bb_chunk_mesh){
        vertex_count,
        vertices,
        index_count,
        indices,
        vertex_array,
        vertex_buffer,
        index_buffer,
    };
}

static void bb_chunk_mesh_destroy(struct bb_chunk_mesh* mesh)
{
    glDeleteVertexArrays(1, &mesh->vertex_array);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->index_buffer);
    free(mesh->vertices);
    free(mesh->indices);
    *mesh = (struct bb_chunk_mesh){0};
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

    gl_object shader_program = bb_shader_create("shaders/basic.glsl");

    struct bb_chunk chunk = {0};
    for (usize y = 27; y < 29; ++y)
    {
        for (usize z = 0; z < bb_chunk_depth; ++z)
        {
            for (usize x = 0; x < bb_chunk_width; ++x)
            {
                if (x < 5)
                    chunk.blocks[y][z][x] = bb_block_grass;
                else if (x < 11)
                    chunk.blocks[y + 1][z][x] = bb_block_grass;
                else
                    chunk.blocks[y + 2][z][x] = bb_block_grass;
            }
        }
    }

    struct bb_chunk_mesh mesh = bb_chunk_mesh_create(&chunk);

    gl_object grass_block_texture = 0;
    glGenTextures(1, &grass_block_texture);
    glBindTexture(GL_TEXTURE_2D, grass_block_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, grass_block_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    mat4 proj = bb_mat4_identity();

    f32 print_timer = 0.0f;
    u32 frame_count = 0;
    while (!bb_was_quit())
    {
        bb_input_update();
        ++frame_count;
        print_timer += bb_delta_time();
        if (print_timer > 1.0f)
        {
            printf("Frame time: %fms\n", print_timer / frame_count * 1.e3f);
            print_timer = 0.0f;
        }

        if (bb_window_resized())
        {
            glViewport(0, 0, bb_window_width(), bb_window_height());
            proj = bb_mat4_perspective((f32)bb_window_width() / (f32)bb_window_height(), 3.14f / 2.0f, 0.1f, 100.0f);
        }

        if (bb_button_held(bb_button_primary) && (bb_mouse_delta().x != 0.0f || bb_mouse_delta().y != 0.0f))
        {
            quat rot_x = bb_quat_from_axis_angle((vec3){0.0f, -1.0f, 0.0f}, bb_mouse_delta().x * 0.003f);
            quat rot_y = bb_quat_from_axis_angle((vec3){-1.0f, 0.0f, 0.0f}, bb_mouse_delta().y * 0.003f);
            player_rotation = bb_quat_mul(rot_x, bb_quat_mul(player_rotation, rot_y));
        }

        vec3 movement = {0};
        movement.x += bb_button_held(bb_button_right) - bb_button_held(bb_button_left);
        movement.y += bb_button_held(bb_button_up) - bb_button_held(bb_button_down);
        movement.z += bb_button_held(bb_button_back) - bb_button_held(bb_button_forward);

        if (movement.x != 0.0f || movement.y != 0.0f || movement.z != 0.0f)
        {
            vec3 rotated = bb_quat_rotate_vec3(player_rotation, (vec3){movement.x, 0.0f, movement.z});
            vec3 dir = bb_vec3_normalize((vec3){rotated.x, movement.y, rotated.z});
            vec3 delta = bb_vec3_mul(dir, 10.0f * bb_delta_time());
            player_position = bb_vec3_add(player_position, delta);
        }

        glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader_program);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glUniformMatrix4fv(glGetUniformLocation(shader_program, "uProj"), 1, false, (f32*)&proj);

        mat4 view = bb_mat4_view(player_position, player_rotation);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "uView"), 1, false, (f32*)&view);

        mat4 model = bb_mat4_identity();
        model.w = (vec4){-8.0f, -32.0f, -8.0f, 1.0f};
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "uModel"), 1, false, (f32*)&model);

        glUniform1i(glGetUniformLocation(shader_program, "uTex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass_block_texture);

        glBindVertexArray(mesh.vertex_array);
        glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, NULL);

        SDL_GL_SwapWindow(window);
    }
}
