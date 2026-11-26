#define GLAD_GL_IMPLEMENTATION
#include "bb.h"
#include "bb_math.h"
#include "bb_input.h"
#include "bb_world.h"
#include "bb_shader.h"

#include <stdio.h>
#include <stddef.h>

static const u32 grass_block_pixels[8][8] = {
    { 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D },
    { 0xFF37804D, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF3E9158 },
    { 0xFF3E9158, 0xFF5AC079, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF37804D },
    { 0xFF37804D, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF3E9158 },
    { 0xFF3E9158, 0xFF429A5E, 0xFF4FAF6F, 0xFF67CB8A, 0xFF4FAF6F, 0xFF5AC079, 0xFF4FAF6F, 0xFF37804D },
    { 0xFF37804D, 0xFF5AC079, 0xFF67CB8A, 0xFF5AC079, 0xFF5AC079, 0xFF4FAF6F, 0xFF67CB8A, 0xFF3E9158 },
    { 0xFF3E9158, 0xFF67CB8A, 0xFF4FAF6F, 0xFF4FAF6F, 0xFF67CB8A, 0xFF5AC079, 0xFF4FAF6F, 0xFF37804D },
    { 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158, 0xFF37804D, 0xFF3E9158 },
};

#define OUTLINE_QUADS 48

static f32 outline_vertices[OUTLINE_QUADS * 4 * 3];
static u32 outline_indices[OUTLINE_QUADS * 6];
static u32 outline_quad_count = 0;

static vec3 bb_outline_corner(i32 dir, i32 a, i32 b, f32 t, f32 va, f32 vb)
{
    f32 components[3];
    components[dir] = t;
    components[a] = va;
    components[b] = vb;
    return (vec3){components[0], components[1], components[2]};
}

static void bb_outline_push_quad(vec3 a, vec3 b, vec3 c, vec3 d, vec3 normal)
{
    if (bb_vec3_dot(bb_vec3_cross(bb_vec3_sub(b, a), bb_vec3_sub(c, a)), normal) < 0.0f)
    {
        vec3 swap = b;
        b = d;
        d = swap;
    }

    vec3 corners[4] = {a, b, c, d};

    u32 base = outline_quad_count * 4;
    f32* out = outline_vertices + base * 3;
    for (u32 corner = 0; corner < 4; ++corner)
    {
        out[corner * 3 + 0] = corners[corner].x;
        out[corner * 3 + 1] = corners[corner].y;
        out[corner * 3 + 2] = corners[corner].z;
    }

    u32* index = outline_indices + outline_quad_count * 6;
    index[0] = base + 0;
    index[1] = base + 1;
    index[2] = base + 2;
    index[3] = base + 0;
    index[4] = base + 2;
    index[5] = base + 3;

    ++outline_quad_count;
}

static void bb_outline_generate(f32 size, f32 thickness)
{
    f32 half_size = size * 0.5f;
    f32 half_thickness = thickness * 0.5f;

    outline_quad_count = 0;

    static const i32 tangent_a[3] = {1, 2, 0};
    static const i32 tangent_b[3] = {2, 0, 1};

    for (i32 dir = 0; dir < 3; ++dir)
    {
        i32 a = tangent_a[dir];
        i32 b = tangent_b[dir];

        for (i32 sign_a = -1; sign_a <= 1; sign_a += 2)
        {
            for (i32 sign_b = -1; sign_b <= 1; sign_b += 2)
            {
                f32 begin = -half_size - half_thickness;
                f32 end = half_size + half_thickness;
                f32 a_range[2] = {sign_a * half_size - half_thickness, sign_a * half_size + half_thickness};
                f32 b_range[2] = {sign_b * half_size - half_thickness, sign_b * half_size + half_thickness};
                vec3 outward_a = {0};
                (&outward_a.x)[a] = (f32)sign_a;
                vec3 outward_b = {0};
                (&outward_b.x)[b] = (f32)sign_b;

                for (i32 shell = 0; shell < 2; ++shell)
                {
                    f32 sign = shell == 0 ? 1.0f : -1.0f;
                    f32 plane_a = sign_a * (half_size + sign * half_thickness);
                    f32 plane_b = sign_b * (half_size + sign * half_thickness);
                    vec3 normal_a = bb_vec3_mul(outward_a, sign);
                    vec3 normal_b = bb_vec3_mul(outward_b, sign);

                    bb_outline_push_quad(
                        bb_outline_corner(dir, a, b, begin, plane_a, b_range[0]),
                        bb_outline_corner(dir, a, b, end, plane_a, b_range[0]),
                        bb_outline_corner(dir, a, b, end, plane_a, b_range[1]),
                        bb_outline_corner(dir, a, b, begin, plane_a, b_range[1]),
                        normal_a);

                    bb_outline_push_quad(
                        bb_outline_corner(dir, a, b, begin, a_range[0], plane_b),
                        bb_outline_corner(dir, a, b, end, a_range[0], plane_b),
                        bb_outline_corner(dir, a, b, end, a_range[1], plane_b),
                        bb_outline_corner(dir, a, b, begin, a_range[1], plane_b),
                        normal_b);
                }
            }
        }
    }
}

#define CROSSHAIR_QUADS 2

static f32 crosshair_vertices[CROSSHAIR_QUADS * 4 * 2];
static u32 crosshair_indices[CROSSHAIR_QUADS * 6];

static void bb_crosshair_generate(f32 length, f32 thickness)
{
    f32 half_length = length * 0.5f;
    f32 half_thickness = thickness * 0.5f;
    static const u32 corner_u[4] = {0, 1, 1, 0};
    static const u32 corner_v[4] = {0, 0, 1, 1};

    for (u32 bar = 0; bar < CROSSHAIR_QUADS; ++bar)
    {
        f32 u_half = bar == 0 ? half_length : half_thickness;
        f32 v_half = bar == 0 ? half_thickness : half_length;

        f32* out = crosshair_vertices + bar * 4 * 2;
        for (u32 corner = 0; corner < 4; ++corner)
        {
            out[corner * 2 + 0] = corner_u[corner] ? u_half : -u_half;
            out[corner * 2 + 1] = corner_v[corner] ? v_half : -v_half;
        }

        u32* index = crosshair_indices + bar * 6;
        index[0] = bar * 4 + 0;
        index[1] = bar * 4 + 1;
        index[2] = bar * 4 + 2;
        index[3] = bar * 4 + 0;
        index[4] = bar * 4 + 2;
        index[5] = bar * 4 + 3;
    }
}

static vec3 player_position = {0};
static quat player_rotation = {1.0f, 0.0f, 0.0f, 0.0f};

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

    gl_object block_shader = bb_shader_create("shaders/blocks.glsl");

    gl_object grass_block_texture = 0;
    glGenTextures(1, &grass_block_texture);
    glBindTexture(GL_TEXTURE_2D, grass_block_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, grass_block_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    struct bb_world world = bb_world_create(8, 8);


    gl_object outline_shader = bb_shader_create("shaders/outline.glsl");
    bb_outline_generate(1.0f, 0.015f);

    gl_object outline_vertex_array = 0;
    glGenVertexArrays(1, &outline_vertex_array);
    glBindVertexArray(outline_vertex_array);

    gl_object outline_vertex_buffer = 0;
    glGenBuffers(1, &outline_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, outline_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outline_vertices), outline_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);

    gl_object outline_index_buffer = 0;
    glGenBuffers(1, &outline_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outline_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(outline_indices), outline_indices, GL_STATIC_DRAW);

    gl_object crosshair_shader = bb_shader_create("shaders/crosshair.glsl");
    bb_crosshair_generate(0.06f, 0.0065f);

    gl_object crosshair_vertex_array = 0;
    glGenVertexArrays(1, &crosshair_vertex_array);
    glBindVertexArray(crosshair_vertex_array);

    gl_object crosshair_vertex_buffer = 0;
    glGenBuffers(1, &crosshair_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, crosshair_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_vertices), crosshair_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), NULL);
    glEnableVertexAttribArray(0);

    gl_object crosshair_index_buffer = 0;
    glGenBuffers(1, &crosshair_index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, crosshair_index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(crosshair_indices), crosshair_indices, GL_STATIC_DRAW);

    mat4 proj = {0};
    {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);
        proj = bb_mat4_perspective((f32)width / (f32)height, 3.14f / 2.0f, 0.1f, 100.0f);
    }

    f32 print_timer = 0.0f;
    u32 frame_count = 0;
    bb_input_update();
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
        struct bb_world_hit hit = bb_world_raycast(&world, (struct bb_ray){player_position, dir});

        if (bb_button_pressed(bb_button_primary) && hit.hit)
        {
            if (hit.hit)
                bb_world_set(&world, hit.pos, bb_block_air);
        }

        if (bb_button_pressed(bb_button_secondary) && hit.hit && hit.face != bb_face_inside)
        {
            vec3 placed_pos = hit.pos;
            switch (hit.face)
            {
            case bb_face_top:
                placed_pos.y += 1.0f;
                break;
            case bb_face_bottom:
                placed_pos.y -= 1.0f;
                break;
            case bb_face_right:
                placed_pos.x += 1.0f;
                break;
            case bb_face_left:
                placed_pos.x -= 1.0f;
                break;
            case bb_face_front:
                placed_pos.z -= 1.0f;
                break;
            case bb_face_back:
                placed_pos.z += 1.0f;
                break;
            case bb_face_inside:
                break;
            }
            bb_world_set(&world, placed_pos, bb_block_grass);
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
        glBindTexture(GL_TEXTURE_2D, grass_block_texture);

        glUniformMatrix4fv(glGetUniformLocation(block_shader, "u_proj"), 1, false, (f32*)&proj);
        glUniformMatrix4fv(glGetUniformLocation(block_shader, "u_view"), 1, false, (f32*)&view);

        for (usize world_z = 0; world_z < world.depth; ++world_z)
        {
            for (usize world_x = 0; world_x < world.width; ++world_x)
            {
                struct bb_chunk_mesh* mesh = &world.chunk_meshes[world_z * world.width + world_x];

                mat4 model = bb_mat4_identity();
                model.w = (vec4){
                    .x = ((f32)world_x - (f32)world.width / 2) * bb_chunk_width,
                    .y = (f32)-bb_chunk_height / 2,
                    .z = ((f32)world_z - (f32)world.depth / 2) * bb_chunk_depth,
                    .w = 1.0f,
                };
                glUniformMatrix4fv(glGetUniformLocation(block_shader, "u_model"), 1, false, (f32*)&model);

                glBindVertexArray(mesh->vertex_array);
                glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, NULL);
            }
        }

        if (hit.hit)
        {
            glUseProgram(outline_shader);

            glUniformMatrix4fv(glGetUniformLocation(outline_shader, "u_proj"), 1, false, (f32*)&proj);
            glUniformMatrix4fv(glGetUniformLocation(outline_shader, "u_view"), 1, false, (f32*)&view);

            mat4 model = bb_mat4_identity();
            model.w = (vec4){
                .x = hit.pos.x,
                .y = hit.pos.y,
                .z = hit.pos.z,
                .w = 1.0f,
            };
            glUniformMatrix4fv(glGetUniformLocation(outline_shader, "u_model"), 1, false, (f32*)&model);

            glBindVertexArray(outline_vertex_array);
            glDrawElements(GL_TRIANGLES, OUTLINE_QUADS * 6, GL_UNSIGNED_INT, NULL);
        }

        glUseProgram(crosshair_shader);
        glDisable(GL_DEPTH_TEST);

        glUniform1f(glGetUniformLocation(crosshair_shader, "u_aspect"), (f32)bb_window_width() / (f32)bb_window_height());

        glBindVertexArray(crosshair_vertex_array);
        glDrawElements(GL_TRIANGLES, CROSSHAIR_QUADS * 6, GL_UNSIGNED_INT, NULL);

        SDL_GL_SwapWindow(window);
    }
}
