#pragma once

#define BSP_LUMP_ENTITIES		0
#define BSP_LUMP_PLANES			1
#define BSP_LUMP_TEXTURES		2
#define BSP_LUMP_VERTICES		3
#define BSP_LUMP_VISIBILITY		4
#define BSP_LUMP_NODES			5
#define BSP_LUMP_TEXINFO		6
#define BSP_LUMP_FACES			7
#define BSP_LUMP_LIGHTING		8
#define BSP_LUMP_CLIPNODES		9
#define BSP_LUMP_LEAFS			10
#define BSP_LUMP_MARKSURFACES	11
#define BSP_LUMP_EDGES			12
#define BSP_LUMP_SURFEDGES		13
#define BSP_LUMP_MODELS			14

#define BSP_LUMP_COUNT			15

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

struct bsp_miptex_lump
{
	s32 MiptexCount;
	s32 DataOffset[4];
};

struct bsp_miptex
{
	char Name[16];
	u32 Width;
	u32 Height;
	u32 Offsets[4];
};

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

struct bsp_tex_info
{
	v3 UAxis;
	f32 UOffset;
	v3 VAxis;
	f32 VOffset;
	s32 Miptex;
	s32 Flags;
};

using bsp_edge = u16[2];

struct bsp_face
{
	s16 Plane;
	s16 Side;
	s32 FirstEdge;
	s16 EdgeCount;
	s16 TexInfo;
	u8 Styles[4];
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