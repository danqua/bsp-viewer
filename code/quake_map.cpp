#define LoadBSP(Filename) ((bsp_header *)LoadBinaryFile(Filename))
#define LoadColorPalette(Filename) ((color *)LoadBinaryFile(Filename))

static map LoadedMap;
static s32 NumIndices;
static u32 *Indices;
static s32 NumRenderFaces;
static s32 *RenderFaces;

#define LIGHTMAP_WIDTH 1024
#define LIGHTMAP_HEIGHT 1024
static s32 Allocated[LIGHTMAP_WIDTH];

bool AllocBlock(s32 Width, s32 Height, s32* X, s32* Y)
{
    s32 Best = LIGHTMAP_HEIGHT;

    for (s32 I = 0; I < LIGHTMAP_WIDTH - Width; I++)
    {
        s32 Best2 = 0;
        s32 J;
        for (J = 0; J < Width; J++)
        {
            if (Allocated[I + J] >= Best) break;
            if (Allocated[I + J] > Best2) Best2 = Allocated[I + J];
        }
        if (J == Width)
        {
            *X = I;
            *Y = Best = Best2;
        }
    }

    if (Best + Height > LIGHTMAP_HEIGHT) return false;

    for (int I = 0; I < Width; I++)
    {
        Allocated[*X + I] = Best + Height;
    }

    return true;
}

static void *LoadBinaryFile(const char *Filename)
{
    FILE* Stream = fopen(Filename, "rb");
    void *Result = 0;

    if (Stream)
    {
        fseek(Stream, 0, SEEK_END);
        size_t FileLength = ftell(Stream);
        fseek(Stream, 0, SEEK_SET);
        Result = malloc(FileLength);
        fread((char *)Result, FileLength, 1, Stream);
        fclose(Stream);
    }
    return Result;
}

static u32 BuildTexture(u8 *MiptexData, s32 Width, s32 Height, s32 Offset,  s32 Pitch, color *ColorPalette, GLenum Filter, color *PixelBuffer)
{
    for (s32 Y = 0; Y < Height; Y++)
    {
        for (s32 X = 0; X < Width; X++)
        {
            s32 MiptexIndex = ((X + Offset) + Y * Pitch);
            s32 PixelIndex = X + Y * Width;
            color Color = ColorPalette[MiptexData[MiptexIndex]];
            PixelBuffer[PixelIndex] = { Color.R, Color.G, Color.B };
        }
    }
    u32 Result = CreateTexture(PixelBuffer, Width, Height, GL_RGB, Filter, GL_REPEAT, 1);
    return Result;
}

template <typename T>
static void CopyLump(bsp_header *Header, s32 LumpType, T **OutItems, s32 *OutNumItems)
{
    bsp_lump Lump = Header->Lumps[LumpType];
    if (OutNumItems) *OutNumItems = Lump.Length / sizeof(T);
    *OutItems = (T *)((u8 *)Header + Lump.Offset);
}

static void MapInitBSP(bsp_header *Header)
{
    CopyLump(Header, BSP_LUMP_PLANES, &LoadedMap.Planes, &LoadedMap.NumPlanes);
    CopyLump(Header, BSP_LUMP_VERTICES, &LoadedMap.Vertices, &LoadedMap.NumVertices);
    CopyLump(Header, BSP_LUMP_NODES, &LoadedMap.Nodes, &LoadedMap.NumNodes);
    CopyLump(Header, BSP_LUMP_TEXINFO, &LoadedMap.TexInfos, &LoadedMap.NumTexInfos);
    CopyLump(Header, BSP_LUMP_FACES, &LoadedMap.Faces, &LoadedMap.NumFaces);
    CopyLump(Header, BSP_LUMP_CLIPNODES, &LoadedMap.ClipNodes, &LoadedMap.NumClipNodes);
    CopyLump(Header, BSP_LUMP_LEAFS, &LoadedMap.Leafs, &LoadedMap.NumLeafs);
    CopyLump(Header, BSP_LUMP_MARKSURFACES, &LoadedMap.MarkSurfaces, &LoadedMap.NumMarkSurfaces);
    CopyLump(Header, BSP_LUMP_EDGES, &LoadedMap.Edges, &LoadedMap.NumEdges);
    CopyLump(Header, BSP_LUMP_SURFEDGES, &LoadedMap.SurfEdges, &LoadedMap.NumSurfEdges);
    CopyLump(Header, BSP_LUMP_MODELS, &LoadedMap.Models, &LoadedMap.NumModels);
    CopyLump(Header, BSP_LUMP_LIGHTMAPS, &LoadedMap.Lightmap, 0);

    bsp_lump MiptexLump = Header->Lumps[BSP_LUMP_MIPTEX];
    LoadedMap.MiptexLump = (bsp_miptex_lump*)((u8 *)Header + MiptexLump.Offset);
}

static void MapInitMaterials()
{
    s32 NumTextures = LoadedMap.MiptexLump->MiptexCount;
    material *Materials = (material *)malloc(sizeof(material) * NumTextures);
    memset(Materials, 0, sizeof(material) * NumTextures);
    LoadedMap.Materials = Materials;
    LoadedMap.NumMaterials = NumTextures;
}

static void MapInitTextures()
{
    color *ColorPalette = LoadColorPalette("quake_palette.pal");
    s32 NumTextures = LoadedMap.MiptexLump->MiptexCount;

    s32 MaxTextureWidth = 256;
    s32 MaxTextureHeight = 256;
    color *PixelBuffer = (color *)malloc(sizeof(color) * MaxTextureWidth * MaxTextureHeight);

    for (s32 TextureIndex = 0; TextureIndex < NumTextures; TextureIndex++)
    {
        material *Material = &LoadedMap.Materials[TextureIndex];
        Material->CullFace = 1;

        bsp_miptex *Miptex = (bsp_miptex *)((u8 *)LoadedMap.MiptexLump + LoadedMap.MiptexLump->DataOffset[TextureIndex]);
        s32 TextureWidth = Miptex->Width;
        s32 TextureHeight = Miptex->Height;
        u8 *MiptexData = (u8 *)Miptex + Miptex->Offsets[0];

        if (strncmp(Miptex->Name, "sky", 3) == 0)
        {
            Material->Program = GetShader("SkyShader");
            Material->DepthTest = true;
            s32 SkyTextureWidth = TextureWidth >> 1;
            u32 ForegroundTexture = BuildTexture(MiptexData, SkyTextureWidth, TextureHeight, 0, TextureWidth, ColorPalette, GL_NEAREST, PixelBuffer);
            u32 BackgroundTexture = BuildTexture(MiptexData, SkyTextureWidth, TextureHeight, SkyTextureWidth, TextureWidth, ColorPalette, GL_NEAREST, PixelBuffer);
            MaterialSetFloat(Material, "Time", 0.0f);
            MaterialSetVec3(Material, "CameraPosition", v3(0.0f));
            MaterialSetTexture(Material, "Texture0", ForegroundTexture);
            MaterialSetTexture(Material, "Texture2", BackgroundTexture);
        }
        else
        {
            u32 Texture = BuildTexture(MiptexData, TextureWidth, TextureHeight, 0, TextureWidth, ColorPalette, GL_LINEAR, PixelBuffer);
            Material->Program = GetShader("SurfaceShader");
            Material->DepthTest = true;
            MaterialSetTexture(Material, "Texture0", Texture);
        }
    }
    free(PixelBuffer);
}

static void CalcSurfaceExtents(surface *Surface)
{
    v2 UVMin = v2(FLT_MAX);
    v2 UVMax = v2(-FLT_MAX);
    bsp_face Face = LoadedMap.Faces[Surface->Face];
    bsp_texinfo TexInfo = LoadedMap.TexInfos[Face.TexInfo];

    for (s32 EdgeIndex = 0; EdgeIndex < Face.EdgeCount; EdgeIndex++)
    {
        s32 Edge = ((s32 *)LoadedMap.SurfEdges)[Face.FirstEdge + EdgeIndex];
        v3 Position = {};

        if (Edge >= 0) Position = LoadedMap.Vertices[LoadedMap.Edges[Edge][0]];
        else           Position = LoadedMap.Vertices[LoadedMap.Edges[-Edge][1]];
        
        f64 U = (f64)Position.x * (f64)TexInfo.UAxis.x +
                (f64)Position.y * (f64)TexInfo.UAxis.y +
                (f64)Position.z * (f64)TexInfo.UAxis.z +
                (f64)TexInfo.UOffset;

        f64 V = (f64)Position.x * (f64)TexInfo.VAxis.x +
                (f64)Position.y * (f64)TexInfo.VAxis.y +
                (f64)Position.z * (f64)TexInfo.VAxis.z +
                (f64)TexInfo.VOffset;
        
        if (UVMin.s > U) UVMin.s = (f32)U;
        if (UVMin.t > V) UVMin.t = (f32)V;

        if (UVMax.s < U) UVMax.s = (f32)U;
        if (UVMax.t < V) UVMax.t = (f32)V;
    }

    for (s32 ComponentIndex = 0; ComponentIndex < 2; ComponentIndex++)
    {
        s32 Min = (s32)glm::floor(UVMin[ComponentIndex] / 16);
        s32 Max = (s32)glm::ceil(UVMax[ComponentIndex] / 16);

        Surface->TextureMins[ComponentIndex] = (Min * 16);
        Surface->UVExtents[ComponentIndex]   = (Max - Min) * 16;
    }
}

static void MarkSurface(s32 LeafIndex)
{
    bsp_leaf Leaf = LoadedMap.Leafs[LeafIndex];
    for (s32 MarkSurfaceIndex = 0; MarkSurfaceIndex < Leaf.MarkSurfaceCount; MarkSurfaceIndex++)
    {
        s32 FaceIndex = LoadedMap.MarkSurfaces[Leaf.FirstMarkSurface + MarkSurfaceIndex];
        RenderFaces[FaceIndex] = 1;
    }
}

static void BuildBSPTree(bsp_node Node)
{
    if ((Node.Children[0] & 0x8000) == 0)
    {
        s32 ChildIndex = Node.Children[0];
        BuildBSPTree(LoadedMap.Nodes[ChildIndex]);
    }
    else
    {
        s32 LeafIndex = ~Node.Children[0];
        MarkSurface(LeafIndex);
    }

    if ((Node.Children[1] & 0x8000) == 0)
    {
        s32 ChildIndex = Node.Children[1];
        BuildBSPTree(LoadedMap.Nodes[ChildIndex]);
    }
    else
    {
        s32 LeafIndex = ~Node.Children[1];
        MarkSurface(LeafIndex);
    }
}


static void CreateSurfaces()
{
    NumRenderFaces = LoadedMap.NumMarkSurfaces;
    RenderFaces = (s32 *)malloc(sizeof(s32) * NumRenderFaces);
    memset(RenderFaces, 0, sizeof(s32) * NumRenderFaces);

    bsp_model Model = LoadedMap.Models[0];
    BuildBSPTree(LoadedMap.Nodes[Model.HeadNodes[0]]);

    LoadedMap.Surfaces = (surface *)malloc(sizeof(surface) * LoadedMap.NumMarkSurfaces);
    
    for (s32 RenderFaceIndex = 0; RenderFaceIndex < NumRenderFaces; RenderFaceIndex++)
    {
        if (RenderFaces[RenderFaceIndex])
        {
            surface *Surface = LoadedMap.Surfaces + RenderFaceIndex;
            *Surface = {};
            Surface->Face = RenderFaceIndex;
            CalcSurfaceExtents(Surface);
            LoadedMap.NumSurfaces++;
        }
    }
}


static s32 GetVertexFromEdge(s32 SurfEdge)
{
    s32 Edge = ((s32 *)LoadedMap.SurfEdges)[SurfEdge];
    if (Edge >= 0)
        return LoadedMap.Edges[Edge][0];
    return LoadedMap.Edges[-Edge][1];
}

static void TriangulateSurface(surface *Surface, u32 *Triangle)
{
    Surface->FirstIndex = NumIndices;
    bsp_face Face = LoadedMap.Faces[Surface->Face];
    s32 NumTriangles = Face.EdgeCount - 2;
    for (s32 Index = 1; Index <= NumTriangles; Index++)
    {
        s32 EdgeIndex = Face.FirstEdge + Index;
        *Triangle++ = GetVertexFromEdge(Face.FirstEdge);
        *Triangle++ = GetVertexFromEdge(EdgeIndex);
        *Triangle++ = GetVertexFromEdge(EdgeIndex + 1);
        Surface->NumIndices += 3;
    }
}

static void MapInitMeshes()
{
    CreateSurfaces();

    // NOTE(daniel): Allocate stuff
    s32 MaxTriangles = 65536;
    Indices = (u32 *)malloc(sizeof(u32) * MaxTriangles);

    u64 SizeInBytes = sizeof(u8) * LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT;
    u8* LightmapBitmap = (u8 *)malloc(SizeInBytes);
    memset(LightmapBitmap, 0, SizeInBytes);

    // NOTE(daniel): Create triangles
    for (s32 SurfaceIndex = 0; SurfaceIndex < LoadedMap.NumSurfaces; SurfaceIndex++)
    {
        surface *Surface = LoadedMap.Surfaces + SurfaceIndex;
        TriangulateSurface(Surface, Indices + NumIndices);
        NumIndices += Surface->NumIndices;
    }

    // NOTE(daniel): Build vertex buffer (position, uv, lightmap)
    u64 MaxVertexBufferSize = sizeof(vertex) * 65536;
    u64 NumVertexBuffers = (u64)LoadedMap.MiptexLump->MiptexCount;

    // NOTE(daniel): A table that holds a list of vertices for each individual miptex. This will result
    // in one vertex buffer per miptex, which will reduce the number of draw calls to the number of textures
    // used inside a map.
    vertex **VertexBuffers = (vertex **)malloc(sizeof(vertex *) * NumVertexBuffers);
    for (s32 VertexBufferIndex = 0; VertexBufferIndex < NumVertexBuffers; VertexBufferIndex++)
    {
        VertexBuffers[VertexBufferIndex] = (vertex *)malloc(sizeof(vertex *) * MaxVertexBufferSize);
    }

    // NOTE(daniel): Keeps track of how many vertices are stored inside the vertex buffers. This will
    // help finding the offsets inside each individual vertex buffer while building the list.
    s32 *VertexBuffersNumVertices = (s32 *)malloc(sizeof(s32) * NumVertexBuffers);
    memset(VertexBuffersNumVertices, 0, sizeof(s32) * NumVertexBuffers);

    for (s32 SurfaceIndex = 0; SurfaceIndex < LoadedMap.NumSurfaces; SurfaceIndex++)
    {
        surface *Surface = LoadedMap.Surfaces + SurfaceIndex;
        bsp_face Face = LoadedMap.Faces[Surface->Face];
        bsp_texinfo TexInfo = LoadedMap.TexInfos[Face.TexInfo];
        bsp_miptex *Miptex = (bsp_miptex *)((u8 *)LoadedMap.MiptexLump + LoadedMap.MiptexLump->DataOffset[TexInfo.Miptex]);
        bsp_vertex *MapVertices = LoadedMap.Vertices;

        s32 VertexBufferIndex = TexInfo.Miptex;
        vertex *VertexBuffer = VertexBuffers[VertexBufferIndex];
        s32 *NumVertices = VertexBuffersNumVertices + VertexBufferIndex;

        s32 LightmapBlockWidth = (Surface->UVExtents.s >> 4) + 1;
        s32 LightmapBlockHeight = (Surface->UVExtents.t >> 4) + 1;
        AllocBlock(LightmapBlockWidth, LightmapBlockHeight, &Surface->LightmapOffset.x, &Surface->LightmapOffset.y);

        u8 *LightmapTexels = LoadedMap.Lightmap + Face.LightOffset;

        for (s32 Y = 0; Y < LightmapBlockHeight; Y++)
        {
            for (s32 X = 0; X < LightmapBlockWidth; X++)
            {
                u8 Pixel = LightmapTexels[X + Y * LightmapBlockWidth];
                u32 ImageIndex = (X+Surface->LightmapOffset.x) + (Y+Surface->LightmapOffset.y) * LIGHTMAP_WIDTH;
                if (Face.LightOffset == -1)
                    LightmapBitmap[ImageIndex] = 255;
                else
                    LightmapBitmap[ImageIndex] = Pixel;
            }
        }
        
        for (s32 Index = 0; Index < Surface->NumIndices; Index++)
        {
            u32 VertexIndex = Indices[Surface->FirstIndex + Index];
            v3 Position = MapVertices[VertexIndex];
            v2 TexCoord = v2(
                (glm::dot(Position, TexInfo.UAxis) + TexInfo.UOffset) / Miptex->Width,
                (glm::dot(Position, TexInfo.VAxis) + TexInfo.VOffset) / Miptex->Height 
            );

            f32 S = glm::dot(Position, TexInfo.UAxis) + TexInfo.UOffset;
            S -= (f32)Surface->TextureMins.s;
            S += (f32)Surface->LightmapOffset.x * 16;
            S += 8;
            S /= (f32)(LIGHTMAP_WIDTH * 16);
            
            f32 T = glm::dot(Position, TexInfo.VAxis) + TexInfo.VOffset;
            T -= (f32)Surface->TextureMins.t;
            T += (f32)Surface->LightmapOffset.y * 16;
            T += 8;
            T /= (f32)(LIGHTMAP_HEIGHT * 16);

            VertexBuffer[*NumVertices].Position = Position;
            VertexBuffer[*NumVertices].TexCoord = TexCoord;
            VertexBuffer[*NumVertices].Lightmap = v2(S, T);
            *NumVertices = *NumVertices + 1;
        }
    }

    LoadedMap.LightmapTexture = CreateTexture(LightmapBitmap, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, GL_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
    free(LightmapBitmap);

    LoadedMap.NumMeshes = NumVertexBuffers;
    LoadedMap.Meshes = (mesh *)malloc(sizeof(mesh) * LoadedMap.NumMeshes);

    for (s32 VertexBufferIndex = 0; VertexBufferIndex < NumVertexBuffers; VertexBufferIndex++)
    {        
        LoadedMap.Meshes[VertexBufferIndex] = CreateMesh(VertexBuffers[VertexBufferIndex], VertexBuffersNumVertices[VertexBufferIndex], 0, 0);
        LoadedMap.Meshes[VertexBufferIndex].Topology = GL_TRIANGLES;
        LoadedMap.Meshes[VertexBufferIndex].MaterialIndex = VertexBufferIndex;
        
        MaterialSetTexture(&LoadedMap.Materials[VertexBufferIndex], "Texture1", LoadedMap.LightmapTexture);
    }

    free(VertexBuffersNumVertices);
    for (s32 VertexBufferIndex = 0; VertexBufferIndex < NumVertexBuffers; VertexBufferIndex++)
    {
        free(VertexBuffers[VertexBufferIndex]);
    }
    free(VertexBuffers);
}

static void LoadMap(const char *Filename)
{
    bsp_header *Header = LoadBSP(Filename);
    MapInitBSP(Header);
    MapInitMaterials();
    MapInitTextures();
    MapInitMeshes();
}

static void DrawMap(f32 Time)
{
    m4x4 ProjectionMatrix = Camera.ProjectionMatrix;
    m4x4 ViewMatrix = CameraGetViewMatrix(&Camera);
    m4x4 QuakeTransformMatrix = m4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,-1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    for (s32 MeshIndex = 0; MeshIndex < LoadedMap.NumMeshes; MeshIndex++)
    {
        mesh Mesh = LoadedMap.Meshes[MeshIndex];
        m4x4 ModelViewProjection = ProjectionMatrix * ViewMatrix * QuakeTransformMatrix;
        material *Material = &LoadedMap.Materials[Mesh.MaterialIndex];
        MaterialBind(*Material);
        MaterialSetFloat(Material, "Time", Time);
        MaterialSetVec3(Material, "CameraPosition", Camera.Position);
        glUniformMatrix4fv(glGetUniformLocation(Material->Program, "ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(ProjectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(Material->Program, "ViewMatrix"), 1, GL_FALSE, glm::value_ptr(ViewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(Material->Program, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(QuakeTransformMatrix));
        MeshDraw(Mesh);
    }
    GuiDrawText(8, 56, "MESHES %d", LoadedMap.NumMeshes);
}