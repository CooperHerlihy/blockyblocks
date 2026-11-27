#include "bb_world.h"

struct bb_block* bb_world_get(struct bb_world* world, i32 x, i32 y, i32 z)
{
    x += (i32)world->width * bb_chunk_width / 2;
    z += (i32)world->depth * bb_chunk_depth / 2;

    if (x < 0 || x >= (i32)world->width * bb_chunk_width ||
        y < 0 || y >= bb_chunk_height ||
        z < 0 || z >= (i32)world->depth * bb_chunk_depth)
        return NULL;

    return &world->chunk_data[z / bb_chunk_width * world->width + x / bb_chunk_width].blocks[y][z % bb_chunk_width][x % bb_chunk_width];
}

struct bb_chunk_vertex {
    vec3 pos;
    vec2 uv;
};

struct bb_chunk_mesh_data {
    usize vertex_capacity;
    usize vertex_count;
    struct bb_chunk_vertex* vertices;

    usize index_capacity;
    usize index_count;
    u32* indices;
};

static void bb_chunk_mesh_add_block(struct bb_chunk_mesh_data* mesh, struct bb_world* world, i32 x, i32 y, i32 z)
{
    struct bb_block* b = bb_world_get(world, x, y, z);
    if (b == NULL || b->type == bb_block_air)
        return;

    struct point {
        i32 x, y, z;
    };

    static const struct point dirs[6] = {
        [0] = {1, 0, 0},
        [1] = {-1, 0, 0},
        [2] = {0, 1, 0},
        [3] = {0, -1, 0},
        [4] = {0, 0, 1},
        [5] = {0, 0, -1},
    };

    static const vec3 faces[6][4] = {
        [0] = {
            {1, 0, 1},
            {1, 0, 0},
            {1, 1, 0},
            {1, 1, 1},
        },
        [1] = {
            {0, 0, 0},
            {0, 0, 1},
            {0, 1, 1},
            {0, 1, 0},
        },
        [2] = {
            {0, 1, 1},
            {1, 1, 1},
            {1, 1, 0},
            {0, 1, 0},
        },
        [3] = {
            {0, 0, 0},
            {1, 0, 0},
            {1, 0, 1},
            {0, 0, 1},
        },
        [4] = {
            {0, 0, 1},
            {1, 0, 1},
            {1, 1, 1},
            {0, 1, 1},
        },
        [5] = {
            {1, 0, 0},
            {0, 0, 0},
            {0, 1, 0},
            {1, 1, 0},
        },
    };

    static const vec2 face_uvs[4] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    };

    vec3 pos = {(f32)x, (f32)y, (f32)z};

    for (usize face = 0; face < 6; ++face)
    {
        struct bb_block* adj = bb_world_get(world, x + dirs[face].x, y + dirs[face].y, z + dirs[face].z);
        if (adj == NULL || adj->type == bb_block_air)
        {
            if (mesh->vertex_count + 4 > mesh->vertex_capacity)
            {
                mesh->vertex_capacity *= 2;
                mesh->vertices = realloc(mesh->vertices, mesh->vertex_capacity * sizeof(*mesh->vertices));
            }
            if (mesh->index_count + 6 > mesh->index_capacity)
            {
                mesh->index_capacity *= 2;
                mesh->indices = realloc(mesh->indices, mesh->index_capacity * sizeof(*mesh->indices));
            }
            mesh->vertices[mesh->vertex_count + 0] = (struct bb_chunk_vertex){
                bb_vec3_add(pos, faces[face][0]),
                face_uvs[0]
            };
            mesh->vertices[mesh->vertex_count + 1] = (struct bb_chunk_vertex){
                bb_vec3_add(pos, faces[face][1]),
                face_uvs[1]
            };
            mesh->vertices[mesh->vertex_count + 2] = (struct bb_chunk_vertex){
                bb_vec3_add(pos, faces[face][2]),
                face_uvs[2]
            };
            mesh->vertices[mesh->vertex_count + 3] = (struct bb_chunk_vertex){
                bb_vec3_add(pos, faces[face][3]),
                face_uvs[3]
            };

            mesh->indices[mesh->index_count + 0] = (u32)mesh->vertex_count + 0;
            mesh->indices[mesh->index_count + 1] = (u32)mesh->vertex_count + 1;
            mesh->indices[mesh->index_count + 2] = (u32)mesh->vertex_count + 2;
            mesh->indices[mesh->index_count + 3] = (u32)mesh->vertex_count + 2;
            mesh->indices[mesh->index_count + 4] = (u32)mesh->vertex_count + 3;
            mesh->indices[mesh->index_count + 5] = (u32)mesh->vertex_count + 0;

            mesh->vertex_count += 4;
            mesh->index_count += 6;
        }
    }
}

static void bb_chunk_mesh_generate(struct bb_world* world, usize chunk_x, usize chunk_z)
{
    struct bb_chunk_mesh_data mesh_data = {0};

    mesh_data.vertex_capacity = 128;
    mesh_data.vertex_count = 0;
    mesh_data.vertices = malloc(mesh_data.vertex_capacity * sizeof(*mesh_data.vertices));

    mesh_data.index_capacity = 128;
    mesh_data.index_count = 0;
    mesh_data.indices = malloc(mesh_data.index_capacity * sizeof(*mesh_data.indices));

    i32 y = 0;
    i32 y_end = bb_chunk_height;
    for (; y < y_end; ++y)
    {
        i32 z = ((i32)chunk_z * bb_chunk_depth - (i32)world->depth * bb_chunk_depth / 2);
        i32 z_end = z + bb_chunk_depth;
        for (; z < z_end; ++z)
        {
            i32 x = ((i32)chunk_x * bb_chunk_width - (i32)world->width * bb_chunk_width / 2);
            i32 x_end = x + bb_chunk_width;
            for (; x < x_end; ++x)
            {
                bb_chunk_mesh_add_block(&mesh_data, world, x, y, z);
            }
        }
    }

    struct bb_mesh* mesh = &world->chunk_meshes[chunk_z * world->width + chunk_x];

    glGenVertexArrays(1, &mesh->vertex_array);
    glBindVertexArray(mesh->vertex_array);

    glGenBuffers(1, &mesh->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, mesh_data.vertex_count * sizeof(*mesh_data.vertices), mesh_data.vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct bb_chunk_vertex), (void*)offsetof(struct bb_chunk_vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct bb_chunk_vertex), (void*)offsetof(struct bb_chunk_vertex, uv));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &mesh->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data.index_count * sizeof(*mesh_data.indices), mesh_data.indices, GL_STATIC_DRAW);

    mesh->index_count = (u32)mesh_data.index_count;

    free(mesh_data.vertices);
    free(mesh_data.indices);
}

static void bb_chunk_mesh_destroy(struct bb_world* world, usize chunk_x, usize chunk_z)
{
    struct bb_mesh* mesh = &world->chunk_meshes[chunk_z * world->width + chunk_x];
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
            struct bb_chunk* chunk = &world.chunk_data[world_z * world.width + world_x];
            memset(chunk->blocks, 0, sizeof(chunk->blocks));
            for (usize y = 0; y < bb_chunk_height / 2; ++y)
            {
                for (usize z = 0; z < bb_chunk_depth; ++z)
                {
                    for (usize x = 0; x < bb_chunk_width; ++x)
                    {
                        chunk->blocks[y][z][x].type = bb_block_grass;
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

void bb_world_update(struct bb_world* world, i32 x, i32 y, i32 z)
{
    x += (i32)world->width * bb_chunk_width / 2;
    z += (i32)world->depth * bb_chunk_depth / 2;

    if (x < 0 || x >= (i32)world->width * bb_chunk_width ||
        y < 0 || y >= bb_chunk_height ||
        z < 0 || z >= (i32)world->depth * bb_chunk_depth)
        return;

    i32 chunk_x = x / bb_chunk_width;
    i32 chunk_z = z / bb_chunk_depth;
    i32 block_x = x % bb_chunk_width;
    i32 block_z = z % bb_chunk_depth;

    bb_chunk_mesh_destroy(world, chunk_x, chunk_z);
    if (block_x == 0 && chunk_x > 0)
        bb_chunk_mesh_destroy(world, chunk_x - 1, chunk_z);
    if (block_x == bb_chunk_width - 1 && chunk_x < (i32)world->width - 1)
        bb_chunk_mesh_destroy(world, chunk_x + 1, chunk_z);
    if (block_z == 0 && chunk_z > 0)
        bb_chunk_mesh_destroy(world, chunk_x, chunk_z - 1);
    if (block_z == bb_chunk_depth - 1 && chunk_z < (i32)world->depth - 1)
        bb_chunk_mesh_destroy(world, chunk_x, chunk_z + 1);

    bb_chunk_mesh_generate(world, chunk_x, chunk_z);
    if (block_x == 0 && chunk_x > 0)
        bb_chunk_mesh_generate(world, chunk_x - 1, chunk_z);
    if (block_x == bb_chunk_width - 1 && chunk_x < (i32)world->width - 1)
        bb_chunk_mesh_generate(world, chunk_x + 1, chunk_z);
    if (block_z == 0 && chunk_z > 0)
        bb_chunk_mesh_generate(world, chunk_x, chunk_z - 1);
    if (block_z == bb_chunk_depth - 1 && chunk_z < (i32)world->depth - 1)
        bb_chunk_mesh_generate(world, chunk_x, chunk_z + 1);
}

bool bb_world_raycast(struct bb_world* world, struct bb_ray ray, struct bb_world_raycast_hit* hit)
{
    i32 x = (i32)floorf(ray.pos.x);
    i32 y = (i32)floorf(ray.pos.y);
    i32 z = (i32)floorf(ray.pos.z);
    struct bb_block* block = bb_world_get(world, x, y, z);
    if (block != NULL && block->type != bb_block_air)
    {
        hit->x = x;
        hit->y = y;
        hit->z = z;
        hit->face = bb_face_inside;
        return true;
    }

    f32 dist = 0.0f;
    for (;;)
    {
        struct bb_box box = bb_unit_cube_around(ray.pos);
        ray.pos.x = fminf(fmaxf(ray.pos.x, box.begin.x + 1.e-4f), box.end.x - 1.e-4f);
        ray.pos.y = fminf(fmaxf(ray.pos.y, box.begin.y + 1.e-4f), box.end.y - 1.e-4f);
        ray.pos.z = fminf(fmaxf(ray.pos.z, box.begin.z + 1.e-4f), box.end.z - 1.e-4f);

        struct bb_raycast ray_hit = bb_raycast_box(ray, box);
        if (!ray_hit.hit)
            break;

        dist += ray_hit.t;
        if (dist > 100.0f)
            break;

        ray.pos = bb_ray_at(ray, ray_hit.t + 1.e-4f);

        x = (i32)floorf(ray.pos.x);
        y = (i32)floorf(ray.pos.y);
        z = (i32)floorf(ray.pos.z);
        block = bb_world_get(world, x, y, z);
        if (block != NULL && block->type != bb_block_air)
        {
            hit->x = x;
            hit->y = y;
            hit->z = z;

            if (ray_hit.norm.x == -1.0f)
            {
                hit->face = bb_face_right;
            }
            else if (ray_hit.norm.x == 1.0f)
            {
                hit->face = bb_face_left;
            }
            else if (ray_hit.norm.y == -1.0f)
            {
                hit->face = bb_face_top;
            }
            else if (ray_hit.norm.y == 1.0f)
            {
                hit->face = bb_face_bottom;
            }
            else if (ray_hit.norm.z == -1.0f)
            {
                hit->face = bb_face_back;
            }
            else if (ray_hit.norm.z == 1.0f)
            {
                hit->face = bb_face_front;
            }

            return true;
        }
    }

    return false;
}
