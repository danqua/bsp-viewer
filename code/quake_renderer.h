#ifndef HARM_RENDERER_H
#define HARM_RENDERER_H

struct plane
{
    f32 Distance;
    v3 Normal;
};

struct camera
{
    f32 Speed;
    f32 Fov;
    f32 Aspect;
    f32 Near;
    f32 Far;
    v3 Position;
    v3 Rotation;
    m4x4 ProjectionMatrix;
};

struct aabb
{
    v3 Min;
    v3 Max;
};

struct vertex
{
    v3 Position;
    v2 TexCoord;
    v2 Lightmap;
};

struct mesh
{
    u32 VAO;
    u32 VBO;
    u32 EBO;
    s32 NumVertices;
    s32 NumIndices;
    s32 Topology;
    s32 MaterialIndex;
    aabb BoundingBox;
};

#define MATERIAL_MAX_UNIFORMS 8
#define MATERIAL_MAX_TEXTURES 4

enum uniform_type
{
    UNIFORM_TYPE_INT,
    UNIFORM_TYPE_FLOAT,
    UNIFORM_TYPE_VEC2,
    UNIFORM_TYPE_VEC3,
    UNIFORM_TYPE_VEC4,
    UNIFORM_TYPE_SAMPLER_2D
};

union uniform_value {
    s32 Int;
    f32 Float;
    v2 Vec2;
    v3 Vec3;
    v4 Vec4;
};

struct uniform
{
    s32 Location;
    uniform_type Type;
    uniform_value Value;
};

struct material
{
    u32 Program;
    s32 NumUniforms;
    uniform Uniforms[MATERIAL_MAX_UNIFORMS];
    b32 Wireframe;
    b32 DepthTest;
    b32 CullFace;
    b32 Blending;
};

#endif