#ifndef MATH_3D_H__
#define MATH_3D_H__
#include <math.h>
#include "helper.h"

// TODO: add our own trig functions,
// I HATE the C standard library

INLINE f32 to_radians(f32 degrees) {
    f32 res = degrees * (PI / 180.0f);
    return(res);
}

typedef union v2
{
    struct { f32 x,y; };
    struct { f32 u,v; };
    struct { f32 r,g; };
    f32 raw[2];
}v2;

INLINE v2  v2m(f32 x, f32 y)    { return (v2){{x, y}}; }
INLINE v2  v2_add(v2 a, v2 b)          { return v2m(a.x+b.x,a.y+b.y); }
INLINE v2  v2_sub(v2 a, v2 b)          { return v2m(a.x-b.x,a.y-b.y); }
INLINE v2  v2_mult(v2 a, v2 b)         { return v2m(a.x*b.x,a.y*b.y); }
INLINE v2  v2_multf(v2 a, f32 b)       { return v2m(a.x*b,a.y*b); }
INLINE v2  v2_div(v2 a, v2 b)          { return v2m(a.x/b.x,a.y/b.y); }
INLINE v2  v2_divf(v2 a, f32 b)        { return v2m(a.x/b,a.y/b); }
INLINE v2  v2_lerp(v2 a, v2 b, f32 x)  { return v2m(a.x*(1.0-x) + b.x*x,a.y*(1.0-x) + b.y*x); }
INLINE f32 v2_dot(v2 a, v2 b)          { return (a.x*b.x)+(a.y*b.y); }
INLINE f32 v2_len(v2 a)                { return sqrtf(v2_dot(a,a)); }
INLINE v2  v2_norm(v2 a)               { f32 vl=v2_len(a);return v2_divf(a,vl); }
INLINE v2  v2_rot(v2 a, f32 angle_rad) { return v2m(a.x*cos(angle_rad)-a.y*sin(angle_rad), a.x*sin(angle_rad)+a.y*cos(angle_rad)); }

typedef union v3
{
    struct { f32 x,y,z; };
    struct { f32 r,g,b; };
    f32 raw[3];
}v3;

INLINE v3 v3m(f32 x, f32 y, f32 z)    { return (v3){{x, y, z}}; }
INLINE v3  v3_add(v3 a, v3 b)         { return v3m(a.x+b.x,a.y+b.y,a.z+b.z); }
INLINE v3  v3_sub(v3 a, v3 b)         { return v3m(a.x-b.x,a.y-b.y,a.z-b.z); }
INLINE v3  v3_mult(v3 a, v3 b)        { return v3m(a.x*b.x,a.y*b.y,a.z*b.z); }
INLINE v3  v3_multf(v3 a, f32 b)      { return v3m(a.x*b,a.y*b,a.z*b); }
INLINE v3  v3_div(v3 a, v3 b)         { return v3m(a.x/b.x,a.y/b.y,a.z/b.z); }
INLINE v3  v3_divf(v3 a, f32 b)       { return v3m(a.x/b,a.y/b,a.z/b); }
INLINE v3  v3_lerp(v3 a, v3 b, f32 x) { return v3m(a.x*(1.0-x) + b.x*x,a.y*(1.0-x) + b.y*x,a.z*(1.0-x)+b.z*x); }
INLINE f32 v3_dot(v3 a, v3 b)         { return (a.x*b.x)+(a.y*b.y)+(a.z*b.z); }
INLINE f32 v3_len(v3 a)               { return sqrtf(v3_dot(a,a)); }
INLINE v3  v3_norm(v3 a)              { f32 vl=v3_len(a);assert(!equalf(vl,0.0,0.01));return v3_divf(a,vl); }
INLINE v3  v3_cross(v3 a,v3 b)        { v3 res; res.x=(a.y*b.z)-(a.z*b.y); res.y=(a.z*b.x)-(a.x*b.z); res.z=(a.x*b.y)-(a.y*b.x); return (res); }

typedef union v4
{
    struct { f32 x,y,z,w; };
    struct { f32 r,g,b,a; };
    f32 raw[4];
}v4;

INLINE v4 v4m(f32 x, f32 y, f32 z, f32 w) { return (v4){{x, y, z, w}}; }
INLINE v4 v4_add(v4 a, v4 b)              { return v4m(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w); }
INLINE v4 v4_sub(v4 a, v4 b)              { return v4m(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w); }
INLINE v4 v4_mult(v4 a, v4 b)             { return v4m(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w); }
INLINE v4 v4_multf(v4 a, f32 b)           { return v4m(a.x*b,a.y*b,a.z*b,a.w*b); }
INLINE v4 v4_div(v4 a, v4 b)              { return v4m(a.x/b.x,a.y/b.y,a.z/b.z,a.w/b.w); }
INLINE v4 v4_divf(v4 a, f32 b)            { return v4m(a.x/b,a.y/b,a.z/b,a.w/b); }

INLINE v4   v4_lerp(v4 a, v4 b, f32 x) { return v4m(a.x*(1.0-x) + b.x*x,a.y*(1.0-x) + b.y*x,a.z*(1.0-x)+b.z*x,a.w*(1.0-x)+b.w*x); }
INLINE f32  v4_dot(v4 a, v4 b)         { return (a.x*b.x)+(a.y*b.y)+(a.z*b.z)+(a.w*b.w); }
INLINE f32  v4_len(v4 a)               { return sqrtf(v4_dot(a,a)); }
INLINE v4   v4_norm(v4 a)              { f32 vl=v4_len(a);assert(!equalf(vl,0.0,0.01));return v4_divf(a,vl); }

typedef union {
    f32 col[3][3];//{x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,p.x,p.y,p.z,1}
    f32 raw[9]; //{x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,p.x,p.y,p.z,1}
} m3;

typedef union {
    f32 col[4][4];//{x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,p.x,p.y,p.z,1}
    f32 raw[16]; //{x.x,x.y,x.z,0,y.x,y.y,y.z,0,z.x,z.y,z.z,0,p.x,p.y,p.z,1}
} m4;

INLINE m4 m4d(f32 d) {
    m4 res = {};
    res.col[0][0] = d;
    res.col[1][1] = d;
    res.col[2][2] = d;
    res.col[3][3] = d;
    return res;
}

INLINE m4 m4_look_at(v3 eye, v3 center, v3 f_up) {
    v3 f = v3_norm(v3_sub(center, eye));
    v3 u = v3_norm(f_up);
    v3 s = v3_norm(v3_cross(f, u));
    u = v3_cross(s, f);

    m4 res = m4d(1.0);
    res.col[0][0] = s.x;
    res.col[1][0] = s.y;
    res.col[2][0] = s.z;
    res.col[0][1] = u.x;
    res.col[1][1] = u.y;
    res.col[2][1] = u.z;
    res.col[0][2] = -f.x;
    res.col[1][2] = -f.y;
    res.col[2][2] = -f.z;
    res.col[3][0] = -v3_dot(s, eye);
    res.col[3][1] = -v3_dot(u, eye);
    res.col[3][2] = v3_dot(f, eye);
    return res;
}

INLINE m4 m4_ortho(f32 l, f32 r, f32 b, f32 t, f32 n, f32 f) {
    m4 res = {};
    res.col[0][0] = 2.0f / (r - l);
    res.col[1][1] = 2.0f / (t - b);
    res.col[2][2] = 2.0f / (n - f);
    res.col[3][3] = 1.0f;
    res.col[3][0] = (l + r) / (l - r);
    res.col[3][1] = (b + t) / (b - t);
    res.col[3][2] = (f + n) / (n - f);
    return res;
}

INLINE v3 m4_extract_pos(m4 m) {
    return v3m(m.col[3][0],m.col[3][1],m.col[3][2]);
}

INLINE m4 m4_scale(v3 s) {
    m4 res = m4d(1.0f);
    res.col[0][0] = s.x;
    res.col[1][1] = s.y;
    res.col[2][2] = s.z;
    return res;
}

INLINE m4 m4_translate(v3 t) {
    m4 res = m4d(1.0f);
    res.col[3][0] = t.x;
    res.col[3][1] = t.y;
    res.col[3][2] = t.z;
    return res;
}

INLINE m4 m4_mult(m4 l, m4 r) {
    m4 res = m4d(1.0f);
    for (u32 col = 0; col < 4; col+=1)
    {
        for (u32 row = 0; row < 4; row+=1)
        {
            f32 sum = 0;
            for (u32 current_index = 0; current_index < 4; ++current_index)
            {
                sum += (f32)l.col[current_index][row] * (f32)r.col[col][current_index];
            }
            res.col[col][row] = sum;
        }
    }
    return res;
}
INLINE v4 m4_multv(m4 mat, v4 vec) {
    v4 res;
    s32 cols, rows;
    for(rows = 0; rows < 4; ++rows)
    {
        f32 s = 0;
        for(cols = 0; cols < 4; ++cols)
        {
            s += mat.col[cols][rows] * vec.raw[cols];
        }
        res.raw[rows] = s;
    }
    return (res);
}

INLINE m4 m4_transpose(m4 m) {
  m4 res = {};
  for (u32 col_idx = 0; col_idx < 4; ++col_idx) {
    for (u32 row_idx = 0; row_idx < 4; ++row_idx) {
      res.col[col_idx][row_idx] = m.col[row_idx][col_idx];
    }
  }
  return res;
}

INLINE m4 m4_inv(m4 m) {
    f32 det;
    m4 inv, inv_out;
    s32 i;

    inv.raw[0] = m.raw[5] * m.raw[10] * m.raw[15] - m.raw[5]  * m.raw[11] * m.raw[14] - m.raw[9]  * m.raw[6]  * m.raw[15] + m.raw[9]  * m.raw[7]  * m.raw[14] + m.raw[13] * m.raw[6]  * m.raw[11] - m.raw[13] * m.raw[7]  * m.raw[10];
    inv.raw[4] = -m.raw[4] * m.raw[10] * m.raw[15] + m.raw[4]  * m.raw[11] * m.raw[14] + m.raw[8]  * m.raw[6]  * m.raw[15] - m.raw[8]  * m.raw[7]  * m.raw[14] - m.raw[12] * m.raw[6]  * m.raw[11] + m.raw[12] * m.raw[7]  * m.raw[10];
    inv.raw[8] = m.raw[4] * m.raw[9] * m.raw[15] - m.raw[4]  * m.raw[11] * m.raw[13] - m.raw[8]  * m.raw[5] * m.raw[15] + m.raw[8]  * m.raw[7] * m.raw[13] + m.raw[12] * m.raw[5] * m.raw[11] - m.raw[12] * m.raw[7] * m.raw[9];
    inv.raw[12] = -m.raw[4] * m.raw[9] * m.raw[14] + m.raw[4]  * m.raw[10] * m.raw[13] + m.raw[8]  * m.raw[5] * m.raw[14] - m.raw[8]  * m.raw[6] * m.raw[13] - m.raw[12] * m.raw[5] * m.raw[10] + m.raw[12] * m.raw[6] * m.raw[9];
    inv.raw[1] = -m.raw[1] * m.raw[10] * m.raw[15] + m.raw[1]  * m.raw[11] * m.raw[14] + m.raw[9]  * m.raw[2] * m.raw[15] - m.raw[9]  * m.raw[3] * m.raw[14] - m.raw[13] * m.raw[2] * m.raw[11] + m.raw[13] * m.raw[3] * m.raw[10];
    inv.raw[5] = m.raw[0] * m.raw[10] * m.raw[15] - m.raw[0]  * m.raw[11] * m.raw[14] - m.raw[8]  * m.raw[2] * m.raw[15] + m.raw[8]  * m.raw[3] * m.raw[14] + m.raw[12] * m.raw[2] * m.raw[11] - m.raw[12] * m.raw[3] * m.raw[10];
    inv.raw[9] = -m.raw[0] * m.raw[9] * m.raw[15] + m.raw[0]  * m.raw[11] * m.raw[13] + m.raw[8]  * m.raw[1] * m.raw[15] - m.raw[8]  * m.raw[3] * m.raw[13] - m.raw[12] * m.raw[1] * m.raw[11] + m.raw[12] * m.raw[3] * m.raw[9];
    inv.raw[13] = m.raw[0] * m.raw[9] * m.raw[14] - m.raw[0]  * m.raw[10] * m.raw[13] - m.raw[8]  * m.raw[1] * m.raw[14] + m.raw[8]  * m.raw[2] * m.raw[13] + m.raw[12] * m.raw[1] * m.raw[10] - m.raw[12] * m.raw[2] * m.raw[9];
    inv.raw[2] = m.raw[1] * m.raw[6] * m.raw[15] - m.raw[1]  * m.raw[7] * m.raw[14] - m.raw[5]  * m.raw[2] * m.raw[15] + m.raw[5]  * m.raw[3] * m.raw[14] + m.raw[13] * m.raw[2] * m.raw[7] - m.raw[13] * m.raw[3] * m.raw[6];
    inv.raw[6] = -m.raw[0] * m.raw[6] * m.raw[15] + m.raw[0]  * m.raw[7] * m.raw[14] + m.raw[4]  * m.raw[2] * m.raw[15] - m.raw[4]  * m.raw[3] * m.raw[14] - m.raw[12] * m.raw[2] * m.raw[7] + m.raw[12] * m.raw[3] * m.raw[6];
    inv.raw[10] = m.raw[0] * m.raw[5] * m.raw[15] - m.raw[0]  * m.raw[7] * m.raw[13] - m.raw[4]  * m.raw[1] * m.raw[15] + m.raw[4]  * m.raw[3] * m.raw[13] + m.raw[12] * m.raw[1] * m.raw[7] - m.raw[12] * m.raw[3] * m.raw[5];
    inv.raw[14] = -m.raw[0] * m.raw[5] * m.raw[14] + m.raw[0]  * m.raw[6] * m.raw[13] + m.raw[4]  * m.raw[1] * m.raw[14] - m.raw[4]  * m.raw[2] * m.raw[13] - m.raw[12] * m.raw[1] * m.raw[6] + m.raw[12] * m.raw[2] * m.raw[5];
    inv.raw[3] = -m.raw[1] * m.raw[6] * m.raw[11] + m.raw[1] * m.raw[7] * m.raw[10] + m.raw[5] * m.raw[2] * m.raw[11] - m.raw[5] * m.raw[3] * m.raw[10] - m.raw[9] * m.raw[2] * m.raw[7] + m.raw[9] * m.raw[3] * m.raw[6];
    inv.raw[7] = m.raw[0] * m.raw[6] * m.raw[11] - m.raw[0] * m.raw[7] * m.raw[10] - m.raw[4] * m.raw[2] * m.raw[11] + m.raw[4] * m.raw[3] * m.raw[10] + m.raw[8] * m.raw[2] * m.raw[7] - m.raw[8] * m.raw[3] * m.raw[6];
    inv.raw[11] = -m.raw[0] * m.raw[5] * m.raw[11] + m.raw[0] * m.raw[7] * m.raw[9] + m.raw[4] * m.raw[1] * m.raw[11] - m.raw[4] * m.raw[3] * m.raw[9] - m.raw[8] * m.raw[1] * m.raw[7] + m.raw[8] * m.raw[3] * m.raw[5];
    inv.raw[15] = m.raw[0] * m.raw[5] * m.raw[10] - m.raw[0] * m.raw[6] * m.raw[9] - m.raw[4] * m.raw[1] * m.raw[10] + m.raw[4] * m.raw[2] * m.raw[9] + m.raw[8] * m.raw[1] * m.raw[6] - m.raw[8] * m.raw[2] * m.raw[5];

    det = m.raw[0] * inv.raw[0] + m.raw[1] * inv.raw[4] + m.raw[2] * inv.raw[8] + m.raw[3] * inv.raw[12];

    if (det == 0) //in case the matrix is non-invertible
        return m4d(0.f);

    det = 1.f / det;

    for (i = 0; i < 16; ++i)
        inv_out.raw[i] = inv.raw[i] * det;

    return inv_out;
}


INLINE m4 mat4_rotate(f32 angle, v3 axis) {
    m4 res = m4d(1.0f);

    axis = v3_norm(axis);

    f32 radians = to_radians(angle);
    f32 sinA = sin(radians);
    f32 cosA = cos(radians);
    f32 t = 1.0f - cosA;

    res.col[0][0] = t * axis.x * axis.x + cosA;
    res.col[0][1] = t * axis.x * axis.y - axis.z * sinA;
    res.col[0][2] = t * axis.x * axis.z + axis.y * sinA;

    res.col[1][0] = t * axis.y * axis.x + axis.z * sinA;
    res.col[1][1] = t * axis.y * axis.y + cosA;
    res.col[1][2] = t * axis.y * axis.z - axis.x * sinA;

    res.col[2][0] = t * axis.z * axis.x - axis.y * sinA;
    res.col[2][1] = t * axis.z * axis.y + axis.x * sinA;
    res.col[2][2] = t * axis.z * axis.z + cosA;

    return res;
}

#endif
