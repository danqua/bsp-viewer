#pragma once

struct map
{
	s32 EntitiyCount;
	s8* Entities;

	s32 PlaneCount;
	bsp_plane* Planes;

	s32 TextureCount;
	bsp_miptex* Textures;

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

	// Rendering info
	render_info RenderInfo;
};