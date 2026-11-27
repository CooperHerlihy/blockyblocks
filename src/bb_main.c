#define GLAD_GL_IMPLEMENTATION
#include "bb.h"
#include "bb_math.h"
#include "bb_input.h"
#include "bb_world.h"
#include "bb_shader.h"

#include <stdio.h>
#include <stddef.h>

static gl_object bb_grass_texture_generate(void)
{
    static const u32 grass_pixels[8][8] = {
        { 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D },
        { 0xFF37804D, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF3E9158 },
        { 0xFF3E9158, 0xFF5AC079, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF37804D },
        { 0xFF37804D, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF3E9158 },
        { 0xFF3E9158, 0xFF429A5E, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF37804D },
        { 0xFF37804D, 0xFF5AC079, 0xFF67CB8A, 0xFF5AC079, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF3E9158 },
        { 0xFF3E9158, 0xFF67CB8A, 0xFF4FAF6F, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF37804D },
        { 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158 },
    };

    gl_object grass_texture = 0;
    glGenTextures(1, &grass_texture);
    glBindTexture(GL_TEXTURE_2D, grass_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, grass_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return grass_texture;
}

static struct bb_mesh bb_outline_generate(f32 thickness)
{
    f32 half_thickness = thickness * 0.5f;

    vec3 axis_dirs[3 * 3] = {
        {half_thickness, 0, 0}, {0, half_thickness, 0}, {0, 0, half_thickness},
        {0, half_thickness, 0}, {0, 0, half_thickness}, {half_thickness, 0, 0},
        {0, 0, half_thickness}, {half_thickness, 0, 0}, {0, half_thickness, 0},
    };

    static const vec3 lines[3 * 4 * 2] = {
        // x axis
        {0, 0, 0}, {1, 0, 0},
        {0, 1, 0}, {1, 1, 0},
        {0, 1, 1}, {1, 1, 1},
        {0, 0, 1}, {1, 0, 1},
        // y axis
        {0, 0, 0}, {0, 1, 0},
        {1, 0, 0}, {1, 1, 0},
        {1, 0, 1}, {1, 1, 1},
        {0, 0, 1}, {0, 1, 1},
        // z axis
        {0, 0, 0}, {0, 0, 1},
        {1, 0, 0}, {1, 0, 1},
        {1, 1, 0}, {1, 1, 1},
        {0, 1, 0}, {0, 1, 1},
    };

    vec3 vertices[3 * 4 * 8] = {0};
    usize vpos = 0;
    u32 indices[3 * 4 * 24] = {0};
    usize ipos = 0;

    for (u32 axis = 0; axis < 3; ++axis)
    {
        vec3 dir = axis_dirs[axis * 3 + 0];
        vec3 norm = axis_dirs[axis * 3 + 1];
        vec3 tan = axis_dirs[axis * 3 + 2];

        for (u32 line = 0; line < 4; ++line)
        {
            vec3 front = bb_vec3_add(lines[axis * 4 * 2 + line * 2 + 1], dir);
            vec3 back = bb_vec3_sub(lines[axis * 4 * 2 + line * 2 + 0], dir);

            vec3 front_top = bb_vec3_add(front, norm);
            vec3 front_bottom = bb_vec3_sub(front, norm);
            vec3 back_top = bb_vec3_add(back, norm);
            vec3 back_bottom = bb_vec3_sub(back, norm);

            vertices[vpos + 0] = bb_vec3_add(front_top, tan);
            vertices[vpos + 1] = bb_vec3_sub(front_top, tan);
            vertices[vpos + 2] = bb_vec3_add(front_bottom, tan);
            vertices[vpos + 3] = bb_vec3_sub(front_bottom, tan);
            vertices[vpos + 4] = bb_vec3_add(back_top, tan);
            vertices[vpos + 5] = bb_vec3_sub(back_top, tan);
            vertices[vpos + 6] = bb_vec3_add(back_bottom, tan);
            vertices[vpos + 7] = bb_vec3_sub(back_bottom, tan);

            // top
            indices[ipos +  0] = (u32)vpos + 0;
            indices[ipos +  1] = (u32)vpos + 1;
            indices[ipos +  2] = (u32)vpos + 5;
            indices[ipos +  3] = (u32)vpos + 5;
            indices[ipos +  4] = (u32)vpos + 4;
            indices[ipos +  5] = (u32)vpos + 0;
            // bottom
            indices[ipos +  6] = (u32)vpos + 2;
            indices[ipos +  7] = (u32)vpos + 6;
            indices[ipos +  8] = (u32)vpos + 7;
            indices[ipos +  9] = (u32)vpos + 7;
            indices[ipos + 10] = (u32)vpos + 3;
            indices[ipos + 11] = (u32)vpos + 2;
            // add
            indices[ipos + 12] = (u32)vpos + 0;
            indices[ipos + 13] = (u32)vpos + 4;
            indices[ipos + 14] = (u32)vpos + 6;
            indices[ipos + 15] = (u32)vpos + 6;
            indices[ipos + 16] = (u32)vpos + 2;
            indices[ipos + 17] = (u32)vpos + 0;
            // sub
            indices[ipos + 18] = (u32)vpos + 1;
            indices[ipos + 19] = (u32)vpos + 3;
            indices[ipos + 20] = (u32)vpos + 7;
            indices[ipos + 21] = (u32)vpos + 7;
            indices[ipos + 22] = (u32)vpos + 5;
            indices[ipos + 23] = (u32)vpos + 1;

            vpos += 8;
            ipos += 24;
        }
    }

    struct bb_mesh outline = {0};

    glGenVertexArrays(1, &outline.vertex_array);
    glBindVertexArray(outline.vertex_array);

    glGenBuffers(1, &outline.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, outline.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &outline.index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    outline.index_count = 3 * 4 * 24;

    return outline;
}

static struct bb_mesh bb_crosshair_generate(f32 length, f32 thickness)
{
    f32 half_length = length * 0.5f;
    f32 half_thickness = thickness * 0.5f;
    vec2 vertices[4 * 2] = {
        {-half_thickness, -half_length},
        { half_thickness, -half_length},
        { half_thickness,  half_length},
        {-half_thickness,  half_length},
        {-half_length, -half_thickness},
        { half_length, -half_thickness},
        { half_length,  half_thickness},
        {-half_length,  half_thickness},
    };

    static const u32 indices[6 * 2] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
    };

    struct bb_mesh crosshair = {0};

    glGenVertexArrays(1, &crosshair.vertex_array);
    glBindVertexArray(crosshair.vertex_array);

    glGenBuffers(1, &crosshair.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, crosshair.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &crosshair.index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, crosshair.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    crosshair.index_count = 6 * 2;

    return crosshair;
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

    SDL_Window* window = SDL_CreateWindow("Blocky Blocks", 800, 600,
        SDL_WINDOW_OPENGL |
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_FULLSCREEN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetWindowRelativeMouseMode(window, true);

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

    struct bb_world world = bb_world_create(8, 8);

    gl_object block_shader = bb_shader_create("shaders/blocks.glsl");
    gl_object grass_texture = bb_grass_texture_generate();

    gl_object outline_shader = bb_shader_create("shaders/outline.glsl");
    struct bb_mesh outline = bb_outline_generate(0.015f);

    gl_object crosshair_shader = bb_shader_create("shaders/crosshair.glsl");
    struct bb_mesh crosshair = bb_crosshair_generate(0.06f, 0.0065f);

    mat4 proj = bb_mat4_identity();
    {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        proj = bb_mat4_perspective((f32)width / (f32)height, 3.14f / 2.0f, 0.1f, 100.0f);
        glViewport(0, 0, width, height);
    }

    vec3 player_position = {0.0f, 66.0f, 0.0f};
    quat player_rotation = {1.0f, 0.0f, 0.0f, 0.0f};

    f32 print_timer = 0.0f;
    u32 frame_count = 0;
    bb_input_update();
    for (;;)
    {
        bb_input_update();
        ++frame_count;
        print_timer += bb_delta_time();
        if (print_timer > 1.0f)
        {
            printf("Frame time: %fms\n", print_timer / frame_count * 1.e3f);
            print_timer = 0.0f;
        }

        if (bb_was_quit())
            break;

        if (bb_window_resized())
        {
            proj = bb_mat4_perspective((f32)bb_window_width() / (f32)bb_window_height(), 3.14f / 2.0f, 0.1f, 100.0f);
            glViewport(0, 0, bb_window_width(), bb_window_height());
        }

        if (bb_mouse_delta().x != 0.0f || bb_mouse_delta().y != 0.0f)
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

        vec3 dir = bb_quat_rotate_vec3(player_rotation, (vec3){0.0f, 0.0f, -1.0f});
        struct bb_world_raycast_hit cast;
        bool facing_block = bb_world_raycast(&world, (struct bb_ray){player_position, dir}, &cast);

        if (bb_button_pressed(bb_button_primary) && facing_block)
        {
            bb_world_get(&world, cast.x, cast.y, cast.z)->type = bb_block_air;
            bb_world_update(&world, cast.x, cast.y, cast.z);
        }

        if (bb_button_pressed(bb_button_secondary) && facing_block && cast.face != bb_face_inside)
        {
            i32 x = cast.x;
            i32 y = cast.y;
            i32 z = cast.z;
            switch (cast.face)
            {
                case bb_face_top:    y += 1; break;
                case bb_face_bottom: y -= 1; break;
                case bb_face_right:  x += 1; break;
                case bb_face_left:   x -= 1; break;
                case bb_face_front:  z -= 1; break;
                case bb_face_back:   z += 1; break;
                case bb_face_inside: break;
            }
            bb_world_get(&world, x, y, z)->type = bb_block_grass;
            bb_world_update(&world, x, y, z);
        }

        glClearColor(0.1f, 0.3f, 0.3f, 1.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 view = bb_mat4_view(player_position, player_rotation);

        glUseProgram(block_shader);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        glUniform1i(glGetUniformLocation(block_shader, "uTex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grass_texture);

        glUniformMatrix4fv(glGetUniformLocation(block_shader, "u_proj"), 1, false, (f32*)&proj);
        glUniformMatrix4fv(glGetUniformLocation(block_shader, "u_view"), 1, false, (f32*)&view);

        for (usize world_z = 0; world_z < world.depth; ++world_z)
        {
            for (usize world_x = 0; world_x < world.width; ++world_x)
            {
                mat4 model = bb_mat4_identity();
                glUniformMatrix4fv(glGetUniformLocation(block_shader, "u_model"), 1, false, (f32*)&model);

                struct bb_mesh* chunk_mesh = &world.chunk_meshes[world_z * world.width + world_x];

                glBindVertexArray(chunk_mesh->vertex_array);
                glDrawElements(GL_TRIANGLES, chunk_mesh->index_count, GL_UNSIGNED_INT, NULL);
            }
        }

        if (facing_block)
        {
            glUseProgram(outline_shader);

            glUniformMatrix4fv(glGetUniformLocation(outline_shader, "u_proj"), 1, false, (f32*)&proj);
            glUniformMatrix4fv(glGetUniformLocation(outline_shader, "u_view"), 1, false, (f32*)&view);

            mat4 model = bb_mat4_identity();
            model.w = (vec4){(f32)cast.x, (f32)cast.y, (f32)cast.z, 1.0f};
            glUniformMatrix4fv(glGetUniformLocation(outline_shader, "u_model"), 1, false, (f32*)&model);

            glBindVertexArray(outline.vertex_array);
            glDrawElements(GL_TRIANGLES, outline.index_count, GL_UNSIGNED_INT, NULL);
        }

        glUseProgram(crosshair_shader);
        glDisable(GL_DEPTH_TEST);

        glUniform1f(glGetUniformLocation(crosshair_shader, "u_aspect"), (f32)bb_window_width() / (f32)bb_window_height());

        glBindVertexArray(crosshair.vertex_array);
        glDrawElements(GL_TRIANGLES, crosshair.index_count, GL_UNSIGNED_INT, NULL);

        SDL_GL_SwapWindow(window);
    }
}
