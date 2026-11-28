#ifndef BB_WORLD_H
#define BB_WORLD_H

#include "bb.h"
#include "bb_math.h"

#define bb_chunk_height 128
#define bb_chunk_width 16
#define bb_chunk_depth 16

enum bb_block_type {
    bb_block_air = 0,
    bb_block_grass,
    bb_block_stone,
    bb_block_count,
};

struct bb_block {
    enum bb_block_type type;
};

struct bb_chunk {
    struct bb_block blocks[bb_chunk_height][bb_chunk_depth][bb_chunk_width];
};

struct bb_world {
    usize width; // The number of chunks in the x direction
    usize depth; // The number of chunks in the y direction
    // depth * width chunks, accessed chunk_data[chunk z idx * width + chunk x idx];
    struct bb_chunk* chunk_data;
    // depth * width chunks, accessed chunk_meshes[chunk z idx * width + chunk x idx];
    struct bb_mesh* chunk_meshes;
    // all the textures in a sprite sheet
    gl_object textures;
};

struct bb_world bb_world_create(usize width, usize depth);
void bb_world_destroy(struct bb_world* world);

// return the block at that position or NULL if there is no block there
struct bb_block* bb_world_get(struct bb_world* world, i32 x, i32 y, i32 z);
// update the chunk(s) containing the position
void bb_world_update(struct bb_world* world, i32 x, i32 y, i32 z);

enum bb_face {
    bb_face_inside,
    bb_face_top,
    bb_face_bottom,
    bb_face_right,
    bb_face_left,
    bb_face_front,
    bb_face_back,
};

struct bb_world_raycast_hit {
    i32 x, y, z;
    enum bb_face face;
};

bool bb_world_raycast(struct bb_world* world, struct bb_ray ray, struct bb_world_raycast_hit* hit);

#endif // BB_WORLD_H
