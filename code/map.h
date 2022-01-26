#pragma once

struct map
{
    s32 EntitiyCount;
    s8* Entities;

    s32 PlaneCount;
    bsp_plane* Planes;

    s32 TextureCount;
    bsp_miptex_lump* MiptexLump;

    s32 VertexCount;
    v3* Vertices;

    s32 VisCount;
    u8* VisData;

    s32 NodeCount;
    bsp_node* Nodes;

    s32 TexInfoCount;
    bsp_tex_info* TexInfos;

    s32 FaceCount;
    bsp_face* Faces;

    s32 LightingCount;
    u8* LightMaps;

    s32 ClipNodeCount;
    bsp_clip_node* ClipNodes;

    s32 LeafCount;
    bsp_leaf* Leafs;

    s32 MarkSurfaceCount;
    u16* MarkSurfaces;

    s32 EdgeCount;
    bsp_edge* Edges;

    s32 SurfEdgeCount;
    s16* SurfEdges;

    s32 ModelCount;
    bsp_model* Models;

};

struct texture
{
    GLuint ID;
    s32 Width;
    s32 Height;
};

struct surface
{
    s32 FirstEdge;
    s32 NumEdges;
    s32 TexInfo;
    s32 VBOOffset;
    u8 Styles[4];
    texture* Texture;

    s16 TextureMins[2];
    s16 Extents[2];

    s32 LightmapS;
    s32 LightmapT;
};

struct image
{
    s32 Width;
    s32 Height;
    s32 XOffset;
    s32 YOffset;
    u8* Pixels;
};

struct vertex
{
    v3 Position;
    v2 UV;
    v2 LightMap;
};

struct render_group
{
    GLuint VAO;
    GLuint VBO;
};

struct game_state
{
    render_group RenderGroup;

    map* Map;

    GLuint LightmapTID;
    GLuint ColorPaletteTID;
};