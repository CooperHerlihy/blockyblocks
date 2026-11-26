#include "bb_world.h"

struct bb_chunk_vertex {
    vec3 pos;
    vec2 uv;
};

static bool bb_world_is_solid(struct bb_world* world, i32 chunk_x, i32 chunk_z, i32 y, i32 z, i32 x)
{
    if (x < 0)
    {
        x += bb_chunk_width;
        --chunk_x;
    }
    else if (x >= bb_chunk_width)
    {
        x -= bb_chunk_width;
        ++chunk_x;
    }
    if (z < 0)
    {
        z += bb_chunk_depth;
        --chunk_z;
    }
    else if (z >= bb_chunk_depth)
    {
        z -= bb_chunk_depth;
        ++chunk_z;
    }

    if (chunk_x < 0 || chunk_x >= (i32)world->width || chunk_z < 0 || chunk_z >= (i32)world->depth)
    {
        return true;
    }

    struct bb_chunk_data* chunk = &world->chunk_data[chunk_z * world->width + chunk_x];
    return chunk->blocks[y][z][x] != bb_block_air;
}

static void bb_chunk_mesh_generate(struct bb_world* world, u32 x_idx, u32 z_idx)
{
    struct bb_chunk_data* chunk = &world->chunk_data[z_idx * world->width + x_idx];

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
                    if (!bb_world_is_solid(world, (i32)x_idx, (i32)z_idx, (i32)y, (i32)z - 1, (i32)x))
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
                    if (!bb_world_is_solid(world, (i32)x_idx, (i32)z_idx, (i32)y, (i32)z + 1, (i32)x))
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
                    if (!bb_world_is_solid(world, (i32)x_idx, (i32)z_idx, (i32)y, (i32)z, (i32)x - 1))
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
                    if (!bb_world_is_solid(world, (i32)x_idx, (i32)z_idx, (i32)y, (i32)z, (i32)x + 1))
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

    struct bb_chunk_mesh* mesh = &world->chunk_meshes[z_idx * world->width + x_idx];

    mesh->index_count = index_count;

    glGenVertexArrays(1, &mesh->vertex_array);
    glBindVertexArray(mesh->vertex_array);

    glGenBuffers(1, &mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(*vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct bb_chunk_vertex), (void*)offsetof(struct bb_chunk_vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct bb_chunk_vertex), (void*)offsetof(struct bb_chunk_vertex, uv));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(*indices), indices, GL_STATIC_DRAW);

    free(vertices);
    free(indices);
}

static void bb_chunk_mesh_destroy(struct bb_world* world, u32 x_idx, u32 z_idx)
{
    struct bb_chunk_mesh* mesh = &world->chunk_meshes[z_idx * world->width + x_idx];
    glDeleteVertexArrays(1, &mesh->vertex_array);
    glDeleteBuffers(1, &mesh->vertex_buffer);
    glDeleteBuffers(1, &mesh->index_buffer);
}

struct bb_world bb_world_create(usize width, usize depth)
{
    struct bb_world world = {0};
    world.width = width;
    world.depth = depth;
    world.chunk_data = malloc(world.width * world.depth * sizeof(*world.chunk_data));
    world.chunk_meshes = malloc(world.width * world.depth * sizeof(*world.chunk_meshes));

    for (usize world_z = 0; world_z < world.depth; ++world_z)
    {
        for (usize world_x = 0; world_x < world.width; ++world_x)
        {
            struct bb_chunk_data* chunk = &world.chunk_data[world_z * world.width + world_x];
            memset(chunk->blocks, 0, sizeof(chunk->blocks));
            for (usize y = 0; y < bb_chunk_height / 2; ++y)
            {
                for (usize z = 0; z < bb_chunk_depth; ++z)
                {
                    for (usize x = 0; x < bb_chunk_width; ++x)
                    {
                        chunk->blocks[y][z][x] = bb_block_grass;
                    }
                }
            }
        }
    }

    for (usize world_z = 0; world_z < world.depth; ++world_z)
    {
        for (usize world_x = 0; world_x < world.width; ++world_x)
        {
            bb_chunk_mesh_generate(&world, world_x, world_z);
        }
    }

    return world;
}

void bb_world_destroy(struct bb_world* world)
{
    for (usize world_z = 0; world_z < world->depth; ++world_z)
    {
        for (usize world_x = 0; world_x < world->width; ++world_x)
        {
            bb_chunk_mesh_destroy(world, world_x, world_z);
        }
    }
}

struct bb_world_coords {
    bool exists;
    usize world_x;
    usize world_z;
    usize chunk_x;
    usize chunk_y;
    usize chunk_z;
};

static struct bb_world_coords bb_world_get_coords(struct bb_world* world, vec3 pos)
{
    i32 half_width = (i32)world->width * bb_chunk_width / 2;
    i32 half_depth = (i32)world->depth * bb_chunk_depth / 2;
    i32 half_height = bb_chunk_height / 2;

    i32 x = (i32)floorf(pos.x + 0.5f) + half_width;
    i32 y = (i32)floorf(pos.y + 0.5f) + half_height;
    i32 z = (i32)floorf(pos.z + 0.5f) + half_depth;

    if (x < 0 || x >= (i32)world->width * bb_chunk_width ||
        y < 0 || y >= bb_chunk_height ||
        z < 0 || z >= (i32)world->depth * bb_chunk_depth)
        return (struct bb_world_coords){0};

    struct bb_world_coords coords = {0};
    coords.exists = true;

    coords.world_x = (usize)(x / bb_chunk_width);
    coords.world_z = (usize)(z / bb_chunk_depth);
    coords.chunk_x = (usize)(x % bb_chunk_width);
    coords.chunk_y = (usize)y;
    coords.chunk_z = (usize)(z % bb_chunk_depth);

    return coords;
}

enum bb_block bb_world_get(struct bb_world* world, vec3 pos)
{
    struct bb_world_coords coords = bb_world_get_coords(world, pos);
    if (!coords.exists)
        return bb_block_air;
    return world->chunk_data[coords.world_z * world->width + coords.world_x].blocks[coords.chunk_y][coords.chunk_z][coords.chunk_x];
}

void bb_world_set(struct bb_world* world, vec3 pos, enum bb_block block)
{
    struct bb_world_coords coords = bb_world_get_coords(world, pos);
    if (!coords.exists)
        return;
    world->chunk_data[coords.world_z * world->width + coords.world_x].blocks[coords.chunk_y][coords.chunk_z][coords.chunk_x] = block;

    bb_chunk_mesh_destroy(world, coords.world_x, coords.world_z);
    if (coords.world_x > 0)
        bb_chunk_mesh_destroy(world, coords.world_x - 1, coords.world_z);
    if (coords.world_x < world->width - 1)
        bb_chunk_mesh_destroy(world, coords.world_x + 1, coords.world_z);
    if (coords.world_z > 0)
        bb_chunk_mesh_destroy(world, coords.world_x, coords.world_z - 1);
    if (coords.world_z < world->depth - 1)
        bb_chunk_mesh_destroy(world, coords.world_x, coords.world_z + 1);

    bb_chunk_mesh_generate(world, coords.world_x, coords.world_z);
    if (coords.world_x > 0)
        bb_chunk_mesh_generate(world, coords.world_x - 1, coords.world_z);
    if (coords.world_x < world->width - 1)
        bb_chunk_mesh_generate(world, coords.world_x + 1, coords.world_z);
    if (coords.world_z > 0)
        bb_chunk_mesh_generate(world, coords.world_x, coords.world_z - 1);
    if (coords.world_z < world->depth - 1)
        bb_chunk_mesh_generate(world, coords.world_x, coords.world_z + 1);
}

struct bb_world_hit bb_world_raycast(struct bb_world* world, struct bb_ray ray)
{
    vec3 cell = {
        floorf(ray.pos.x + 0.5f),
        floorf(ray.pos.y + 0.5f),
        floorf(ray.pos.z + 0.5f),
    };

    if (bb_world_get(world, cell) != bb_block_air)
    {
        struct bb_world_hit hit;
        hit.pos = cell;
        hit.face = bb_face_inside;
        hit.hit = true;
        return hit;
    }

    f32 dist = 0.0f;
    for (usize i = 0; i < 200; ++i)
    {
        struct bb_box box = bb_unit_cube_at(cell);
        ray.pos.x = fminf(fmaxf(ray.pos.x, box.begin.x + 1.e-4f), box.end.x - 1.e-4f);
        ray.pos.y = fminf(fmaxf(ray.pos.y, box.begin.y + 1.e-4f), box.end.y - 1.e-4f);
        ray.pos.z = fminf(fmaxf(ray.pos.z, box.begin.z + 1.e-4f), box.end.z - 1.e-4f);

        struct bb_raycast ray_hit = bb_raycast_box(ray, box);
        if (!ray_hit.hit)
            break;

        dist += ray_hit.t;
        if (dist > 100.0f)
            break;

        ray.pos = bb_ray_at(ray, ray_hit.t);
        cell = bb_vec3_add(cell, ray_hit.norm);

        if (bb_world_get(world, cell) != bb_block_air)
        {
            struct bb_world_hit world_hit;
            world_hit.hit = true;
            world_hit.pos = cell;

            vec3 norm = bb_vec3_mul(ray_hit.norm, -1.0f);
            if (norm.x == 1.0f)
            {
                world_hit.face = bb_face_right;
            }
            else if (norm.x == -1.0f)
            {
                world_hit.face = bb_face_left;
            }
            else if (norm.y == 1.0f)
            {
                world_hit.face = bb_face_top;
            }
            else if (norm.y == -1.0f)
            {
                world_hit.face = bb_face_bottom;
            }
            else if (norm.z == 1.0f)
            {
                world_hit.face = bb_face_back;
            }
            else if (norm.z == -1.0f)
            {
                world_hit.face = bb_face_front;
            }

            return world_hit;
        }
    }

    return (struct bb_world_hit){0};
}
