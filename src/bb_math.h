#ifndef BB_MATH_H
#define BB_MATH_H

#include "bb.h"

#include <math.h>

typedef struct vec2 {
    f32 x, y;
} vec2;

static inline vec2 bb_vec2_add(vec2 l, vec2 r)
{
    return (vec2){l.x + r.x, l.y + r.y};
}

static inline vec2 bb_vec2_sub(vec2 l, vec2 r)
{
    return (vec2){l.x - r.x, l.y - r.y};
}

static inline vec2 bb_vec2_mul(vec2 l, f32 r)
{
    return (vec2){l.x * r, l.y * r};
}

static inline vec2 bb_vec2_div(vec2 l, f32 r)
{
    return (vec2){l.x / r, l.y / r};
}

static inline f32 bb_vec2_dot(vec2 l, vec2 r)
{
    return l.x * r.x + l.y * r.y;
}

static inline f32 bb_vec2_length(vec2 v)
{
    return sqrtf(bb_vec2_dot(v, v));
}

static inline vec2 bb_vec2_normalize(vec2 v)
{
    return bb_vec2_div(v, bb_vec2_length(v));
}

static inline f32 bb_vec2_cross(vec2 l, vec2 r)
{
    return l.x * r.y - l.y * r.x;
}

typedef struct vec3 {
    f32 x, y, z;
} vec3;

static inline vec3 bb_vec3_add(vec3 l, vec3 r)
{
    return (vec3){l.x + r.x, l.y + r.y, l.z + r.z};
}

static inline vec3 bb_vec3_sub(vec3 l, vec3 r)
{
    return (vec3){l.x - r.x, l.y - r.y, l.z - r.z};
}

static inline vec3 bb_vec3_mul(vec3 l, f32 r)
{
    return (vec3){l.x * r, l.y * r, l.z * r};
}

static inline vec3 bb_vec3_div(vec3 l, f32 r)
{
    return (vec3){l.x / r, l.y / r, l.z / r};
}

static inline f32 bb_vec3_dot(vec3 l, vec3 r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z;
}

static inline f32 bb_vec3_length(vec3 v)
{
    return sqrtf(bb_vec3_dot(v, v));
}

static inline vec3 bb_vec3_normalize(vec3 v)
{
    return bb_vec3_div(v, bb_vec3_length(v));
}

static inline vec3 bb_vec3_cross(vec3 l, vec3 r)
{
    return (vec3){l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z, l.x * r.y - l.y * r.x};
}

typedef struct vec4 {
    f32 x, y, z, w;
} vec4;

static inline vec4 bb_vec4_add(vec4 l, vec4 r)
{
    return (vec4){l.x + r.x, l.y + r.y, l.z + r.z, l.w + r.w};
}

static inline vec4 bb_vec4_sub(vec4 l, vec4 r)
{
    return (vec4){l.x - r.x, l.y - r.y, l.z - r.z, l.w - r.w};
}

static inline vec4 bb_vec4_mul(vec4 l, f32 r)
{
    return (vec4){l.x * r, l.y * r, l.z * r, l.w * r};
}

static inline vec4 bb_vec4_div(vec4 l, f32 r)
{
    return (vec4){l.x / r, l.y / r, l.z / r, l.w / r};
}

static inline f32 bb_vec4_dot(vec4 l, vec4 r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z + l.w * r.w;
}

static inline f32 bb_vec4_length(vec4 v)
{
    return sqrtf(bb_vec4_dot(v, v));
}

static inline vec4 bb_vec4_normalize(vec4 v)
{
    return bb_vec4_div(v, bb_vec4_length(v));
}

/**
 * Matrices are column major for use with OpenGL
 */

typedef struct mat2 {
    vec2 x, y;
} mat2;

static inline mat2 bb_mat2_identity(void)
{
    return (mat2){{1.0f, 0.0f}, {0.0f, 1.0f}};
}

static inline mat2 bb_mat2_add(mat2 l, mat2 r)
{
    return (mat2){bb_vec2_add(l.x, r.x), bb_vec2_add(l.y, r.y)};
}

static inline mat2 bb_mat2_sub(mat2 l, mat2 r)
{
    return (mat2){bb_vec2_sub(l.x, r.x), bb_vec2_sub(l.y, r.y)};
}

static inline mat2 bb_mat2_mul(mat2 l, mat2 r)
{
    return (mat2){
        (vec2){l.x.x * r.x.x + l.y.x * r.x.y, l.x.y * r.x.x + l.y.y * r.x.y},
        (vec2){l.x.x * r.y.x + l.y.x * r.y.y, l.x.y * r.y.x + l.y.y * r.y.y},
    };
}

static inline vec2 bb_mat2_mul_vec(mat2 l, vec2 r)
{
    return (vec2){l.x.x * r.x + l.y.x * r.y, l.x.y * r.x + l.y.y * r.y};
}

static inline mat2 bb_mat2_mul_scalar(mat2 l, f32 r)
{
    return (mat2){bb_vec2_mul(l.x, r), bb_vec2_mul(l.y, r)};
}

static inline mat2 bb_mat2_div_scalar(mat2 l, f32 r)
{
    return (mat2){bb_vec2_div(l.x, r), bb_vec2_div(l.y, r)};
}

typedef struct mat3 {
    vec3 x, y, z;
} mat3;

static inline mat3 bb_mat3_identity(void)
{
    return (mat3){{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
}

static inline mat3 bb_mat3_add(mat3 l, mat3 r)
{
    return (mat3){bb_vec3_add(l.x, r.x), bb_vec3_add(l.y, r.y), bb_vec3_add(l.z, r.z)};
}

static inline mat3 bb_mat3_sub(mat3 l, mat3 r)
{
    return (mat3){bb_vec3_sub(l.x, r.x), bb_vec3_sub(l.y, r.y), bb_vec3_sub(l.z, r.z)};
}

static inline mat3 bb_mat3_mul(mat3 l, mat3 r)
{
    return (mat3){
        (vec3){
            l.x.x * r.x.x + l.y.x * r.x.y + l.z.x * r.x.z,
            l.x.y * r.x.x + l.y.y * r.x.y + l.z.y * r.x.z,
            l.x.z * r.x.x + l.y.z * r.x.y + l.z.z * r.x.z,
        },
        (vec3){
            l.x.x * r.y.x + l.y.x * r.y.y + l.z.x * r.y.z,
            l.x.y * r.y.x + l.y.y * r.y.y + l.z.y * r.y.z,
            l.x.z * r.y.x + l.y.z * r.y.y + l.z.z * r.y.z,
        },
        (vec3){
            l.x.x * r.z.x + l.y.x * r.z.y + l.z.x * r.z.z,
            l.x.y * r.z.x + l.y.y * r.z.y + l.z.y * r.z.z,
            l.x.z * r.z.x + l.y.z * r.z.y + l.z.z * r.z.z,
        },
    };
}

static inline vec3 bb_mat3_mul_vec(mat3 l, vec3 r)
{
    return (vec3){
        l.x.x * r.x + l.y.x * r.y + l.z.x * r.z,
        l.x.y * r.x + l.y.y * r.y + l.z.y * r.z,
        l.x.z * r.x + l.y.z * r.y + l.z.z * r.z,
    };
}

static inline mat3 bb_mat3_mul_scalar(mat3 l, f32 r)
{
    return (mat3){bb_vec3_mul(l.x, r), bb_vec3_mul(l.y, r), bb_vec3_mul(l.z, r)};
}

static inline mat3 bb_mat3_div_scalar(mat3 l, f32 r)
{
    return (mat3){bb_vec3_div(l.x, r), bb_vec3_div(l.y, r), bb_vec3_div(l.z, r)};
}

typedef struct mat4 {
    vec4 x, y, z, w;
} mat4;

static inline mat4 bb_mat4_identity(void)
{
    return (mat4){
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
}

static inline mat4 bb_mat4_add(mat4 l, mat4 r)
{
    return (mat4){
        bb_vec4_add(l.x, r.x),
        bb_vec4_add(l.y, r.y),
        bb_vec4_add(l.z, r.z),
        bb_vec4_add(l.w, r.w),
    };
}

static inline mat4 bb_mat4_sub(mat4 l, mat4 r)
{
    return (mat4){
        bb_vec4_sub(l.x, r.x),
        bb_vec4_sub(l.y, r.y),
        bb_vec4_sub(l.z, r.z),
        bb_vec4_sub(l.w, r.w),
    };
}

static inline mat4 bb_mat4_mul(mat4 l, mat4 r)
{
    return (mat4){
        (vec4){
            l.x.x * r.x.x + l.y.x * r.x.y + l.z.x * r.x.z + l.w.x * r.x.w,
            l.x.y * r.x.x + l.y.y * r.x.y + l.z.y * r.x.z + l.w.y * r.x.w,
            l.x.z * r.x.x + l.y.z * r.x.y + l.z.z * r.x.z + l.w.z * r.x.w,
            l.x.w * r.x.x + l.y.w * r.x.y + l.z.w * r.x.z + l.w.w * r.x.w,
        },
        (vec4){
            l.x.x * r.y.x + l.y.x * r.y.y + l.z.x * r.y.z + l.w.x * r.y.w,
            l.x.y * r.y.x + l.y.y * r.y.y + l.z.y * r.y.z + l.w.y * r.y.w,
            l.x.z * r.y.x + l.y.z * r.y.y + l.z.z * r.y.z + l.w.z * r.y.w,
            l.x.w * r.y.x + l.y.w * r.y.y + l.z.w * r.y.z + l.w.w * r.y.w,
        },
        (vec4){
            l.x.x * r.z.x + l.y.x * r.z.y + l.z.x * r.z.z + l.w.x * r.z.w,
            l.x.y * r.z.x + l.y.y * r.z.y + l.z.y * r.z.z + l.w.y * r.z.w,
            l.x.z * r.z.x + l.y.z * r.z.y + l.z.z * r.z.z + l.w.z * r.z.w,
            l.x.w * r.z.x + l.y.w * r.z.y + l.z.w * r.z.z + l.w.w * r.z.w,
        },
        (vec4){
            l.x.x * r.w.x + l.y.x * r.w.y + l.z.x * r.w.z + l.w.x * r.w.w,
            l.x.y * r.w.x + l.y.y * r.w.y + l.z.y * r.w.z + l.w.y * r.w.w,
            l.x.z * r.w.x + l.y.z * r.w.y + l.z.z * r.w.z + l.w.z * r.w.w,
            l.x.w * r.w.x + l.y.w * r.w.y + l.z.w * r.w.z + l.w.w * r.w.w,
        },
    };
}

static inline vec4 bb_mat4_mul_vec(mat4 l, vec4 r)
{
    return (vec4){
        l.x.x * r.x + l.y.x * r.y + l.z.x * r.z + l.w.x * r.w,
        l.x.y * r.x + l.y.y * r.y + l.z.y * r.z + l.w.y * r.w,
        l.x.z * r.x + l.y.z * r.y + l.z.z * r.z + l.w.z * r.w,
        l.x.w * r.x + l.y.w * r.y + l.z.w * r.z + l.w.w * r.w,
    };
}

static inline mat4 bb_mat4_mul_scalar(mat4 l, f32 r)
{
    return (mat4){
        bb_vec4_mul(l.x, r),
        bb_vec4_mul(l.y, r),
        bb_vec4_mul(l.z, r),
        bb_vec4_mul(l.w, r),
    };
}

static inline mat4 bb_mat4_div_scalar(mat4 l, f32 r)
{
    return (mat4){
        bb_vec4_div(l.x, r),
        bb_vec4_div(l.y, r),
        bb_vec4_div(l.z, r),
        bb_vec4_div(l.w, r),
    };
}

typedef struct quat {
    f32 r, i, j, k;
} quat;

static inline quat bb_quat_identity(void)
{
    return (quat){1.0f, 0.0f, 0.0f, 0.0f};
}

static inline quat bb_quat_add(quat l, quat r)
{
    return (quat){l.r + r.r, l.i + r.i, l.j + r.j, l.k + r.k};
}

static inline quat bb_quat_sub(quat l, quat r)
{
    return (quat){l.r - r.r, l.i - r.i, l.j - r.j, l.k - r.k};
}

static inline quat bb_quat_mul(quat l, quat r)
{
    return (quat){
        l.r * r.r - l.i * r.i - l.j * r.j - l.k * r.k,
        l.r * r.i + l.i * r.r + l.j * r.k - l.k * r.j,
        l.r * r.j - l.i * r.k + l.j * r.r + l.k * r.i,
        l.r * r.k + l.i * r.j - l.j * r.i + l.k * r.r,
    };
}

static inline quat bb_quat_conjugate(quat q)
{
    return (quat){q.r, -q.i, -q.j, -q.k};
}

static inline f32 bb_quat_length(quat q)
{
    return sqrtf(q.r * q.r + q.i * q.i + q.j * q.j + q.k * q.k);
}

static inline quat bb_quat_normalize(quat q)
{
    f32 len = bb_quat_length(q);
    return (quat){q.r / len, q.i / len, q.j / len, q.k / len};
}

/**
 * Axis must be normalized, angle in radians
 */
static inline quat bb_quat_from_axis_angle(vec3 axis, f32 angleRad)
{
    f32 half = angleRad * 0.5f;
    f32 s = sinf(half);
    return (quat){cosf(half), axis.x * s, axis.y * s, axis.z * s};
}

/**
 * Rotation taking normalized vector a to normalized vector b
 */
static inline quat bb_quat_from_difference(vec3 a, vec3 b)
{
    vec3 half = bb_vec3_normalize(bb_vec3_add(a, b));
    f32 r = bb_vec3_dot(a, half);
    return bb_quat_normalize((quat){r, a.y * half.z - a.z * half.y, a.z * half.x - a.x * half.z, a.x * half.y - a.y * half.x});
}

static inline vec3 bb_quat_rotate_vec3(quat q, vec3 v)
{
    quat c = bb_quat_conjugate(q);
    quat p = (quat){0.0f, v.x, v.y, v.z};
    quat t = bb_quat_mul(bb_quat_mul(q, p), c);
    return (vec3){t.i, t.j, t.k};
}

static inline vec4 bb_quat_rotate_vec4(quat q, vec4 v)
{
    quat c = bb_quat_conjugate(q);
    quat p = (quat){0.0f, v.x, v.y, v.z};
    quat t = bb_quat_mul(bb_quat_mul(q, p), c);
    return (vec4){t.i, t.j, t.k, v.w};
}

static inline mat3 bb_quat_rotate_mat3(quat q, mat3 m)
{
    return (mat3){
        bb_quat_rotate_vec3(q, m.x),
        bb_quat_rotate_vec3(q, m.y),
        bb_quat_rotate_vec3(q, m.z),
    };
}

static inline mat4 bb_quat_rotate_mat4(quat q, mat4 m)
{
    return (mat4){
        bb_quat_rotate_vec4(q, m.x),
        bb_quat_rotate_vec4(q, m.y),
        bb_quat_rotate_vec4(q, m.z),
        bb_quat_rotate_vec4(q, m.w),
    };
}

static inline mat4 bb_mat4_perspective(f32 aspect, f32 fovRad, f32 nearPlane, f32 farPlane)
{
    f32 f = 1.0f / tanf(fovRad * 0.5f);
    return (mat4){
        (vec4){f / aspect, 0.0f, 0.0f, 0.0f},
        (vec4){0.0f, f, 0.0f, 0.0f},
        (vec4){0.0f, 0.0f, (farPlane + nearPlane) / (nearPlane - farPlane), -1.0f},
        (vec4){0.0f, 0.0f, (2.0f * farPlane * nearPlane) / (nearPlane - farPlane), 0.0f},
    };
}

static inline mat4 bb_mat4_view(vec3 position, quat rotation)
{
    mat4 ret = {0};
    ret.x.x = 1.0f;
    ret.y.y = 1.0f;
    ret.z.z = 1.0f;
    ret.w = (vec4){-position.x, -position.y, -position.z, 1.0f};
    ret = bb_quat_rotate_mat4(bb_quat_conjugate(rotation), ret);
    return ret;
}

#endif // BB_MATH_H
