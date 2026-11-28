#include "bb_world.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

    static const vec2 texture_uvs[bb_block_count][4] = {
        [bb_block_grass] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.5f}, {0.0f, 0.5f}},
        [bb_block_stone] = {{0.0f, 0.5f}, {1.0f, 0.5f}, {1.0f, 1.0f}, {0.0f, 1.0f}}
    };

    struct bb_block* b = bb_world_get(world, x, y, z);
    if (b == NULL || b->type == bb_block_air)
        return;

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

            const vec2* uvs = texture_uvs[b->type];
            mesh->vertices[mesh->vertex_count + 0] = (struct bb_chunk_vertex){bb_vec3_add(pos, faces[face][0]), uvs[0]};
            mesh->vertices[mesh->vertex_count + 1] = (struct bb_chunk_vertex){bb_vec3_add(pos, faces[face][1]), uvs[1]};
            mesh->vertices[mesh->vertex_count + 2] = (struct bb_chunk_vertex){bb_vec3_add(pos, faces[face][2]), uvs[2]};
            mesh->vertices[mesh->vertex_count + 3] = (struct bb_chunk_vertex){bb_vec3_add(pos, faces[face][3]), uvs[3]};

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

static gl_object bb_world_texture_generate(void)
{
    int width, height, channels;
    void* pixels = stbi_load("resources/textures.png", &width, &height, &channels, 4);
    if (pixels == NULL)
    {
        printf("Could not load \"resources/textures.png\"");
        width = 1;
        height = 1;
        pixels = malloc(sizeof(u32));
        ((u32*)pixels)[0] = 0xFFFF00FF;
    }

    gl_object texture = 0;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    free(pixels);
    return texture;
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
            for (usize y = 0; y < bb_chunk_height; ++y)
            {
                for (usize z = 0; z < bb_chunk_depth; ++z)
                {
                    for (usize x = 0; x < bb_chunk_width; ++x)
                    {
                        enum bb_block_type type = bb_block_air;
                        if (y < 61)
                            type = bb_block_stone;
                        else if (y < 64)
                            type = bb_block_grass;
                        chunk->blocks[y][z][x].type = type;
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

    world.textures = bb_world_texture_generate();

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

bool bb_world_raycast(struct bb_world* world, struct bb_ray ray, struct bb_world_raycast_hit* cast)
{
    i32 x = (i32)floorf(ray.pos.x);
    i32 y = (i32)floorf(ray.pos.y);
    i32 z = (i32)floorf(ray.pos.z);

    struct bb_block* block = bb_world_get(world, x, y, z);
    if (block != NULL && block->type != bb_block_air)
    {
        cast->x = x;
        cast->y = y;
        cast->z = z;
        cast->face = bb_face_inside;
        return true;
    }

    i32 x_sign = (i32)(ray.dir.x > 0) - (i32)(ray.dir.x < 0);
    i32 y_sign = (i32)(ray.dir.y > 0) - (i32)(ray.dir.y < 0);
    i32 z_sign = (i32)(ray.dir.z > 0) - (i32)(ray.dir.z < 0);

    f32 x_t = x_sign > 0 ? ((x + 1) - ray.pos.x) / ray.dir.x : x_sign < 0 ? (x - ray.pos.x) / ray.dir.x : INFINITY;
    f32 y_t = y_sign > 0 ? ((y + 1) - ray.pos.y) / ray.dir.y : y_sign < 0 ? (y - ray.pos.y) / ray.dir.y : INFINITY;
    f32 z_t = z_sign > 0 ? ((z + 1) - ray.pos.z) / ray.dir.z : z_sign < 0 ? (z - ray.pos.z) / ray.dir.z : INFINITY;

    f32 x_d = x_sign / ray.dir.x;
    f32 y_d = y_sign / ray.dir.y;
    f32 z_d = z_sign / ray.dir.z;

    for (usize i = 0; i < 100; ++i)
    {
        if (x_t < y_t)
        {
            if (x_t < z_t)
            {
                x += x_sign;
                block = bb_world_get(world, x, y, z);
                if (block == NULL)
                    return false;
                if (block->type != bb_block_air)
                {
                    cast->x = x;
                    cast->y = y;
                    cast->z = z;
                    cast->face = x_sign > 0 ? bb_face_left : bb_face_right;
                    return true;
                }
                x_t += x_d;
            }
            else
            {
                z += z_sign;
                block = bb_world_get(world, x, y, z);
                if (block == NULL)
                    return false;
                if (block->type != bb_block_air)
                {
                    cast->x = x;
                    cast->y = y;
                    cast->z = z;
                    cast->face = z_sign > 0 ? bb_face_front : bb_face_back;
                    return true;
                }
                z_t += z_d;
            }
        }
        else
        {
            if (y_t < z_t)
            {
                y += y_sign;
                block = bb_world_get(world, x, y, z);
                if (block == NULL)
                    return false;
                if (block->type != bb_block_air)
                {
                    cast->x = x;
                    cast->y = y;
                    cast->z = z;
                    cast->face = y_sign > 0 ? bb_face_bottom : bb_face_top;
                    return true;
                }
                y_t += y_d;
            }
            else
            {
                z += z_sign;
                block = bb_world_get(world, x, y, z);
                if (block == NULL)
                    return false;
                if (block->type != bb_block_air)
                {
                    cast->x = x;
                    cast->y = y;
                    cast->z = z;
                    cast->face = z_sign > 0 ? bb_face_front : bb_face_back;
                    return true;
                }
                z_t += z_d;
            }
        }
    }

    return false;
}
