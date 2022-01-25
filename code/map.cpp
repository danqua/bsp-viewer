#define GRID_SIZE 16.0f
#define LIGHTMAP_WIDTH 512
#define LIGHTMAP_HEIGHT 512

static GLuint LightMapTextureID;
static GLuint ColorPalette;

static bsp_header* BSPHeader = nullptr;

static texture* Textures;

static map* LoadedMap = nullptr;
static surface* Surfaces;

static s32 Allocated[LIGHTMAP_WIDTH];

std::vector<image> Images;
static u8 LightmapImage[LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT];

static GLuint CreateColorPalette()
{
    FILE* File = fopen("PALETTE.pal", "rb");
    fseek(File, 0, SEEK_END);
    size_t Length = ftell(File);
    rewind(File);
    u8* Buffer = (u8*)malloc(Length);
    Assert(Buffer);
    fread(Buffer, 1, Length, File);
    fclose(File);

    glGenTextures(1, &ColorPalette);
    glBindTexture(GL_TEXTURE_2D, ColorPalette);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, GL_NONE, GL_RGB, GL_UNSIGNED_BYTE, Buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    free(Buffer);
    return ColorPalette;
}

bool AllocBlock(s32 Width, s32 Height, s32* X, s32* Y)
{
    s32 Best = LIGHTMAP_HEIGHT;

    for (s32 I = 0; I < LIGHTMAP_WIDTH - Width; I++)
    {
        s32 Best2 = 0;
        s32 J;
        for (J = 0; J < Width; J++)
        {
            if (Allocated[I + J] >= Best)
                break;
            if (Allocated[I + J] > Best2)
                Best2 = Allocated[I + J];
        }
        if (J == Width)
        {
            *X = I;
            *Y = Best = Best2;
        }
    }

    if (Best + Height > LIGHTMAP_HEIGHT)
        return false;

    for (int i = 0; i < Width; i++)
        Allocated[*X + i] = Best + Height;

    return true;
}

template <typename T>
inline s32 CopyLump(T** Field, s32 Lump)
{
    s8* RawData = (s8*)BSPHeader;
    s32 Count = BSPHeader->Lumps[Lump].Length / (s32)sizeof(T);
    *Field = (T*)(RawData + BSPHeader->Lumps[Lump].Offset);
    return Count;
}

inline s32 CopyTextureLump(bsp_miptex_lump** MiptexLump)
{
    s8* RawData = (s8*)BSPHeader;
    *MiptexLump = (bsp_miptex_lump*)(RawData + BSPHeader->Lumps[BSP_LUMP_TEXTURES].Offset);
    return (*MiptexLump)->MiptexCount;
}



static bool LoadMap(map* Result, const char* Filename)
{
    if (BSPHeader) {
        free(BSPHeader);
        BSPHeader = nullptr;
    }

    FILE* File = fopen(Filename, "rb");
    if (!File) return false;
    fseek(File, 0, SEEK_END);
    size_t Length = ftell(File);
    rewind(File);

    BSPHeader = (bsp_header*)malloc(Length);
    Assert(BSPHeader);

    fread(BSPHeader, 1, Length, File);
    fclose(File);

    Result->EntitiyCount        = CopyLump(&Result->Entities,     BSP_LUMP_ENTITIES);
    Result->PlaneCount          = CopyLump(&Result->Planes,       BSP_LUMP_PLANES);
    Result->VertexCount	        = CopyLump(&Result->Vertices,     BSP_LUMP_VERTICES);
    Result->VisCount            = CopyLump(&Result->VisData,      BSP_LUMP_VISIBILITY);
    Result->NodeCount           = CopyLump(&Result->Nodes,        BSP_LUMP_NODES);
    Result->TexInfoCount        = CopyLump(&Result->TexInfos,     BSP_LUMP_TEXINFO);
    Result->FaceCount           = CopyLump(&Result->Faces,        BSP_LUMP_FACES);
    Result->LightingCount       = CopyLump(&Result->LightMaps,    BSP_LUMP_LIGHTING);
    Result->ClipNodeCount       = CopyLump(&Result->ClipNodes,    BSP_LUMP_CLIPNODES);
    Result->LeafCount           = CopyLump(&Result->Leafs,        BSP_LUMP_LEAFS);
    Result->MarkSurfaceCount    = CopyLump(&Result->MarkSurfaces, BSP_LUMP_MARKSURFACES);
    Result->EdgeCount           = CopyLump(&Result->Edges,        BSP_LUMP_EDGES);
    Result->SurfEdgeCount       = CopyLump(&Result->SurfEdges,    BSP_LUMP_SURFEDGES);
    Result->ModelCount          = CopyLump(&Result->Models,       BSP_LUMP_MODELS);

    // Initialize textures
    Result->TextureCount        = CopyTextureLump(&Result->MiptexLump);
    Textures = (texture*)malloc(sizeof(texture) * Result->TextureCount);
    Assert(memset(Textures, 0, sizeof(texture) * Result->TextureCount));

    for (s32 I = 0; I < Result->TextureCount; I++)
    {
        if (Result->MiptexLump->DataOffset[I] == 0)
        {
            // Load externally
            s32 TextureNotInWad = 0;
        }
        else
        {
            bsp_miptex* Miptex = (bsp_miptex*)(((s8*)Result->MiptexLump) + Result->MiptexLump->DataOffset[I]);
            u8* Pixels = (u8*)Miptex + Miptex->Offsets[0];
            texture* Texture = &Textures[I];
            Texture->Width = Miptex->Width;
            Texture->Height = Miptex->Height;
            Texture->ID = OpenGLCreateTexture(Pixels, Texture->Width, Texture->Height, GL_RED);
        }
    }



    Result->RenderInfo.VAO = 0;
    Result->RenderInfo.PositionVBO = 0;
    Result->RenderInfo.UVVBO = 0;
    Result->RenderInfo.LightMapVBO = 0;

    LoadedMap = Result;

    Surfaces = (surface*)malloc(LoadedMap->FaceCount * sizeof(surface));

    static u8 EmptyPixels[LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT] = {};
    for (s32 I = 0; I < LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT; I++)
        EmptyPixels[I] = 255;

    // Create texture atlas for lightmap
    LightMapTextureID = OpenGLCreateTexture(0, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, GL_RED);
    OpenGLTextureFilter(LightMapTextureID, GL_LINEAR);
    return true;
}


static void GetMiptexSize(const bsp_face* Face, s32* Width, s32* Height)
{
    bsp_tex_info* TexInfo = &LoadedMap->TexInfos[Face->TexInfo];
    bsp_miptex* Miptex = (bsp_miptex*)(((s8*)LoadedMap->MiptexLump) + LoadedMap->MiptexLump->DataOffset[TexInfo->Miptex]);
    *Width = Miptex->Width;
    *Height = Miptex->Height;
}

static void CalcSurfaceExtents(surface* Surface)
{
    v2 UVMin = v2(FLT_MAX);
    v2 UVMax = v2(-FLT_MAX);

    bsp_tex_info* TexInfo = &LoadedMap->TexInfos[Surface->TexInfo];

    for (s32 I = 0; I < Surface->NumEdges; I++)
    {
        s32 Index = ((s32*)LoadedMap->SurfEdges)[Surface->FirstEdge + I];
        v3 Position;

        if (Index >= 0)
            Position = LoadedMap->Vertices[LoadedMap->Edges[Index][0]];
        else
            Position = LoadedMap->Vertices[LoadedMap->Edges[-Index][1]];
        

        f64 U = (f64)Position.x * (f64)TexInfo->UAxis.x +
                (f64)Position.y * (f64)TexInfo->UAxis.y +
                (f64)Position.z * (f64)TexInfo->UAxis.z +
                (f64)TexInfo->UOffset;

        f64 V = (f64)Position.x * (f64)TexInfo->VAxis.x +
                (f64)Position.y * (f64)TexInfo->VAxis.y +
                (f64)Position.z * (f64)TexInfo->VAxis.z +
                (f64)TexInfo->VOffset;

        if (U < UVMin[0]) UVMin[0] = (f32)U;
        if (V < UVMin[1]) UVMin[1] = (f32)V;

        if (U > UVMax[0]) UVMax[0] = (f32)U;
        if (V > UVMax[1]) UVMax[1] = (f32)V;
    }

    for (s32 I = 0; I < 2; I++)
    {
        s32 Min = (s32)floor(UVMin[I] / 16);
        s32 Max = (s32)ceil(UVMax[I] / 16);
        Surface->TextureMins[I] = Min * 16;
        Surface->Extents[I] = (Max - Min) * 16;
    }
}
bool loaded = false;
static void CreateSurfaces(map* Map)
{
    surface* Surface = Surfaces;

    for (bsp_face* Face = Map->Faces; Face != &Map->Faces[Map->FaceCount]; Face++, Surface++)
    {
        Surface->FirstEdge = Face->FirstEdge;
        Surface->NumEdges = Face->EdgeCount;
        Surface->TexInfo = Face->TexInfo;
        CalcSurfaceExtents(Surface);
        Surface->LightmapS = 0;
        Surface->LightmapT = 0;
        
        s32 Width = (Surface->Extents[0] >> 4) + 1;
        s32 Height = (Surface->Extents[1] >> 4) + 1;
        AllocBlock(Width, Height, &Surface->LightmapS, &Surface->LightmapT);

        if (Face->LightOffset != -1)
        {
            image Image;
            Image.Width = Width;
            Image.Height = Height;
            Image.XOffset = Surface->LightmapS;
            Image.YOffset = Surface->LightmapT;
            Image.Pixels = &LoadedMap->LightMaps[Face->LightOffset];
            Images.push_back(Image);
            //glBindTexture(GL_TEXTURE_2D, LightMapTextureID);
            //glTexSubImage2D(GL_TEXTURE_2D, 0, Surface->LightmapS, Surface->LightmapT, Width, Height, GL_RED, GL_UNSIGNED_BYTE, &LoadedMap->LightMaps[Face->LightOffset]);
        }

        /*
        s32 NumVertices = Face->EdgeCount;
        bsp_tex_info* TexInfo = &Map->TexInfos[Face->TexInfo];
        s32 MiptexWidth, MiptexHeight;
        GetMiptexSize(Face, &MiptexWidth, &MiptexHeight);
        
        for (s32 I = 0; I < NumVertices; I++)
        {
            s32 Index = Map->SurfEdges[Face->FirstEdge + I];
            v3 Position;

            if (Index > 0) Position = Map->Vertices[Map->Edges[Index][0]];
            else Position = Map->Vertices[Map->Edges[-Index][1]];

            // Texture
            f32 U = glm::dot(Position, TexInfo->UAxis) + TexInfo->UOffset;
            U /= MiptexWidth;
            
            f32 V = glm::dot(Position, TexInfo->VAxis) + TexInfo->VOffset;
            V /= MiptexHeight;

            // Lightmap
            f32 S = glm::dot(Position, TexInfo->UAxis) + TexInfo->UOffset;
            S -= (f32)Surface->TextureMins[0];
            S += (f32)Surface->LightmapS * 16.0f;
            S += 8;
            S /= LIGHTMAP_WIDTH * 16;
            
            f32 T = glm::dot(Position, TexInfo->VAxis) + TexInfo->VOffset;
            S -= (f32)Surface->TextureMins[1];
            S += (f32)Surface->LightmapT * 16.0f;
            S += 8;
            S /= LIGHTMAP_WIDTH * 16;
        }
        */
    }

    {
        for (const image& Image : Images)
        {
            for (s32 Y = 0; Y < Image.Height; Y++)
            {
                u8* Source = &Image.Pixels[Y * Image.Width];
                u8* Destination = LightmapImage + ((Image.YOffset + Y) * LIGHTMAP_WIDTH+ Image.XOffset);
                std::copy(Source, Source + Image.Width, Destination);
            }
        }

        glBindTexture(GL_TEXTURE_2D, LightMapTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, GL_NONE, GL_RED, GL_UNSIGNED_BYTE, LightmapImage);

    }
}

static void CreateVertexBuffer(map* Map)
{
    CreateSurfaces(Map);

    s32 VertexCount = 0;
    for (bsp_face* Face = Map->Faces; Face != &Map->Faces[Map->FaceCount]; Face++)
        VertexCount += Face->EdgeCount;

    vertex* Vertices = (vertex*)malloc(sizeof(vertex) * VertexCount);
    vertex* Vertex = Vertices;
    surface* Surface = Surfaces;

    for (bsp_face* Face = LoadedMap->Faces; Face != &LoadedMap->Faces[LoadedMap->FaceCount]; Face++, Surface++)
    {
        s32 NumVertices = Face->EdgeCount;
        bsp_tex_info* TexInfo = &LoadedMap->TexInfos[Face->TexInfo];

        s32 MiptexWidth, MiptexHeight;
        GetMiptexSize(Face, &MiptexWidth, &MiptexHeight);

        for (s32 I = 0; I < NumVertices; I++, Vertex++)
        {
            s32 Index = ((s32*)LoadedMap->SurfEdges)[Face->FirstEdge + I];

            if (Index > 0)
            {
                s16 Edge = LoadedMap->Edges[Index][0];
                Vertex->Position = LoadedMap->Vertices[Edge];
            }
            else
            {
                s16 Edge = LoadedMap->Edges[-Index][1];
                Vertex->Position = LoadedMap->Vertices[Edge];
            }

            // Texture
            {
                f32 S = glm::dot(Vertex->Position, TexInfo->UAxis) + TexInfo->UOffset;
                S /= MiptexWidth;

                f32 T = glm::dot(Vertex->Position, TexInfo->VAxis) + TexInfo->VOffset;
                T /= MiptexHeight;
                
                Vertex->UV = v2(S, T);
            }

            // Lightmap
            {
                f32 S = glm::dot(Vertex->Position, TexInfo->UAxis) + TexInfo->UOffset;
                S -= (f32)Surface->TextureMins[0];
                S += (f32)(Surface->LightmapS * 16);
                S += 8;
                S /= (f32)(LIGHTMAP_WIDTH * 16);

                f32 T = glm::dot(Vertex->Position, TexInfo->VAxis) + TexInfo->VOffset;
                T -= (f32)Surface->TextureMins[1];
                T += (f32)(Surface->LightmapT * 16);
                T += 8;
                T /= (f32)(LIGHTMAP_HEIGHT * 16);

                Vertex->LightMap = v2(S, T);
            }
        }
    }

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * VertexCount, Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)(offsetof(vertex, Position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)(offsetof(vertex, UV)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void*)(offsetof(vertex, LightMap)));

    Map->RenderInfo.VAO = VAO;
    Map->RenderInfo.PositionVBO = VBO;

    free(Vertices);
}