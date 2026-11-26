#ifndef BB_WORLD_H
#define BB_WORLD_H

#include "bb.h"
#include "bb_math.h"

#define bb_chunk_height 128
#define bb_chunk_width 16
#define bb_chunk_depth 16

// The block types
enum bb_block {
    bb_block_air = 0,
    bb_block_grass,
    bb_block_count,
};

struct bb_chunk_data {
    enum bb_block blocks[bb_chunk_height][bb_chunk_depth][bb_chunk_width];
};

struct bb_chunk_mesh {
    gl_object vertex_array;
    gl_object vertex_buffer;
    gl_object index_buffer;
    u32 index_count;
};

struct bb_world {
    usize width; // The number of chunks in the x direction
    usize depth; // The number of chunks in the y direction
    // depth * width chunks, accessed chunk_data[chunk z idx * width + chunk x idx];
    struct bb_chunk_data* chunk_data;
    // depth * width chunks, accessed chunk_meshes[chunk z idx * width + chunk x idx];
    struct bb_chunk_mesh* chunk_meshes;
};

struct bb_world bb_world_create(usize width, usize depth);
void bb_world_destroy(struct bb_world* world);

enum bb_block bb_world_get(struct bb_world* world, vec3 pos);
void bb_world_set(struct bb_world* world, vec3 pos, enum bb_block block);

enum bb_face {
    bb_face_inside,
    bb_face_top,
    bb_face_bottom,
    bb_face_right,
    bb_face_left,
    bb_face_front,
    bb_face_back,
};

struct bb_world_hit {
    vec3 pos;
    enum bb_face face;
    bool hit;
};

struct bb_world_hit bb_world_raycast(struct bb_world* world, struct bb_ray ray);

#endif // BB_WORLD_H
