#ifndef MATH_3D_H__
#define MATH_3D_H__
#include <math.h>
#include "helper.h"

// TODO: add our own trig functions,
// I HATE the C standard library

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

#endif
