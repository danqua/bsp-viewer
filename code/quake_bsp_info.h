#ifndef HARM_BSP_INFO_H
#define HARM_BSP_INFO_H

#define BSP_LUMP_ENTITIES       0
#define BSP_LUMP_PLANES	        1
#define BSP_LUMP_MIPTEX         2
#define BSP_LUMP_VERTICES       3
#define BSP_LUMP_VISIBILITY     4
#define BSP_LUMP_NODES          5
#define BSP_LUMP_TEXINFO        6
#define BSP_LUMP_FACES          7
#define BSP_LUMP_LIGHTMAPS      8
#define BSP_LUMP_CLIPNODES      9
#define BSP_LUMP_LEAFS          10
#define BSP_LUMP_MARKSURFACES   11  // List of faces
#define BSP_LUMP_EDGES          12
#define BSP_LUMP_SURFEDGES      13
#define BSP_LUMP_MODELS         14

#define BSP_LUMP_COUNT          15

struct bsp_lump
{
    s32 Offset;
    s32 Length;
};

struct bsp_header
{
    s32 Version;
    bsp_lump Lumps[BSP_LUMP_COUNT];
};

struct bsp_model
{
    v3 Min;
    v3 Max;
    v3 Origin;
    s32 HeadNodes[4];
    s32 VisLeafs;
    s32 FirstFace;
    s32 NumFaces;
};

typedef v3 bsp_vertex;

struct bsp_miptex_lump
{
    s32 MiptexCount;
    s32 DataOffset[4];
};

#define MIPLEVELS 4
struct bsp_miptex
{
    char Name[16];
    u32 Width;
    u32 Height;
    u32 Offsets[MIPLEVELS];
};

#define PLANE_X    0
#define PLANE_Y    1
#define PLANE_Z    2
#define PLANE_ANYX 3
#define PLANE_ANYY 4
#define PLANE_ANYZ 5

struct bsp_plane
{
    v3 Normal;
    f32 Distance;
    s32 Type;
};

struct bsp_node
{
    s32 Plane;
    s16 Children[2];
    s16 Min[3];
    s16 Max[3];
    u16 FirstFace;
    u16 FaceCount;
};

struct bsp_clip_node
{
    s32 Plane;
    s32 Children[2];
};

struct bsp_texinfo
{
    v3 UAxis;
    f32 UOffset;
    v3 VAxis;
    f32 VOffset;
    s32 Miptex;
    s32 Flags;
};
#define	TEX_SPECIAL     1 // sky or slime, no lightmap or 256 subdivision
#define TEX_MISSING     2 // this texinfo does not have a texture

typedef u16 bsp_edge[2];

#define	MAXLIGHTMAPS    4
struct bsp_face
{
    s16 Plane;
    s16 Side;
    s32 FirstEdge;
    s16 EdgeCount;
    s16 TexInfo;
    u8 Styles[MAXLIGHTMAPS];
    s32 LightOffset;
};

struct bsp_leaf
{
    s32 Contents;
    s32 VisOffset;
    s16 Min[3];
    s16 Max[3];
    u16 FirstMarkSurface;
    u16 MarkSurfaceCount;
    u8 AmbientLevel[4];
};

#endif