#ifndef HARM_MAP_H
#define HARM_MAP_H

struct color
{
    u8 R;
    u8 G;
    u8 B;
};

struct surface
{
    s32 Face;
    s32 NumIndices;
    s32 FirstIndex;
    glm::i16vec2 TextureMins;
    glm::i16vec2 UVExtents;
    glm::ivec2 LightmapOffset;
};


#define MAP_MAX_SKY_TEXTURES 8
struct sky_texture
{
    s32 Miptex;
    GLuint Foreground;
    GLuint Background;
};

enum face_type
{
    FACE_SOLID,
    FACE_SKY
};

struct render_group
{
    s32 NumFaces;
    s32 *Faces;
    s32 FaceType;
};

struct map
{
    s32 NumMeshes;
    mesh *Meshes;

    GLuint Program;
    GLuint *Textures;
    GLuint LightmapTexture;

    s32 NumMaterials;
    material *Materials;

    s32 NumSurfaces;
    surface *Surfaces;

    s32 NumPlanes;
    bsp_plane *Planes;

    s32 NumVertices;
    bsp_vertex *Vertices;

    s32 NumNodes;
    bsp_node *Nodes;

    s32 NumTexInfos;
    bsp_texinfo *TexInfos;

    s32 NumFaces;
    bsp_face *Faces;

    s32 NumClipNodes;
    bsp_clip_node *ClipNodes;

    s32 NumLeafs;
    bsp_leaf *Leafs;

    s32 NumMarkSurfaces;
    u16 *MarkSurfaces;

    s32 NumEdges;
    bsp_edge *Edges;

    s32 NumSurfEdges;
    s16 *SurfEdges;

    s32 NumModels;
    bsp_model *Models;

    u8 *Lightmap;

    bsp_miptex_lump *MiptexLump;
    bsp_miptex *Miptex;
};

#endif