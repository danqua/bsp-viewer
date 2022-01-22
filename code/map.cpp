static bsp_header* BSPHeader = nullptr;

template <typename T>
inline s32 CopyLump(T** Field, s32 Lump)
{
	s8* RawData = (s8*)BSPHeader;
	s32 Count = BSPHeader->Lumps[Lump].Length / (s32)sizeof(T);
	*Field = (T*)(RawData + BSPHeader->Lumps[Lump].Offset);
	return Count;
}

bool LoadMap(map* Result, const char* Filename)
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

	Result->EntitiyCount		= CopyLump(&Result->Entities,		BSP_LUMP_ENTITIES);
	Result->PlaneCount			= CopyLump(&Result->Planes,			BSP_LUMP_PLANES);
	Result->TextureCount		= CopyLump(&Result->Textures,		BSP_LUMP_TEXTURES);
	Result->VertexCount			= CopyLump(&Result->Vertices,		BSP_LUMP_VERTICES);
	Result->VisCount			= CopyLump(&Result->VisData,		BSP_LUMP_VISIBILITY);
	Result->NodeCount			= CopyLump(&Result->Nodes,			BSP_LUMP_NODES);
	Result->TexInfoCount		= CopyLump(&Result->TexInfos,		BSP_LUMP_TEXINFO);
	Result->FaceCount			= CopyLump(&Result->Faces,			BSP_LUMP_FACES);
	Result->LightingCount		= CopyLump(&Result->LightMaps,		BSP_LUMP_LIGHTING);
	Result->ClipNodeCount		= CopyLump(&Result->ClipNodes,		BSP_LUMP_CLIPNODES);
	Result->LeafCount			= CopyLump(&Result->Leafs,			BSP_LUMP_LEAFS);
	Result->MarkSurfaceCount	= CopyLump(&Result->MarkSurfaces,	BSP_LUMP_MARKSURFACES);
	Result->EdgeCount			= CopyLump(&Result->Edges,			BSP_LUMP_EDGES);
	Result->SurfEdgeCount		= CopyLump(&Result->SurfEdges,		BSP_LUMP_SURFEDGES);
	Result->ModelCount			= CopyLump(&Result->Models,			BSP_LUMP_MODELS);

	Result->RenderInfo.VAO = 0;
	Result->RenderInfo.PositionVBO = 0;
	Result->RenderInfo.NormalVBO = 0;
	Result->RenderInfo.UVVBO = 0;

	return true;
}

void CreateVertexBuffer(map* Map)
{
	// Get the number of vertices needed from all faces
	s32 VertexCount = 0;
	for (bsp_face* Face = Map->Faces; Face != &Map->Faces[Map->FaceCount]; Face++)
		VertexCount += Face->EdgeCount;

	// Allocate and fill the buffers
	v3* VertexBuffer = (v3*)malloc(sizeof * VertexBuffer * VertexCount);
	v3* NormalBuffer = (v3*)malloc(sizeof * NormalBuffer * VertexCount);
	v2* UVBuffer = (v2*)malloc(sizeof * UVBuffer * VertexCount);

	v3* Vertex = VertexBuffer;
	v3* Normal = NormalBuffer;
	v2* UV = UVBuffer;

	for (bsp_face* Face = Map->Faces; Face != &Map->Faces[Map->FaceCount]; Face++)
	{
		bsp_tex_info* TexInfo = &Map->TexInfos[Face->TexInfo];

		for (s32 SurfEdge = Face->FirstEdge; SurfEdge < Face->FirstEdge + Face->EdgeCount; SurfEdge++, Vertex++, Normal++, UV++)
		{
			s32 Edge = ((s32*)Map->SurfEdges)[SurfEdge];

			*Vertex = Map->Vertices[Edge > 0 ? Map->Edges[Edge][0] : Map->Edges[-Edge][1]];
			*Normal = Map->Planes[Face->Plane].Normal;

			UV->s = glm::dot(*Vertex / 32.0f, TexInfo->UAxis) + TexInfo->UOffset;
			UV->t = glm::dot(*Vertex / 32.0f, TexInfo->VAxis) + TexInfo->VOffset;
		}
	}

	// Allocate GPU memory
	glGenVertexArrays(1, &Map->RenderInfo.VAO);
	glBindVertexArray(Map->RenderInfo.VAO);

	glGenBuffers(1, &Map->RenderInfo.PositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, Map->RenderInfo.PositionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v3) * VertexCount, VertexBuffer, GL_STATIC_DRAW);

	glGenBuffers(1, &Map->RenderInfo.NormalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, Map->RenderInfo.NormalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v3) * VertexCount, NormalBuffer, GL_STATIC_DRAW);

	glGenBuffers(1, &Map->RenderInfo.UVVBO);
	glBindBuffer(GL_ARRAY_BUFFER, Map->RenderInfo.UVVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(v2) * VertexCount, UVBuffer, GL_STATIC_DRAW);

	free(UVBuffer);
	free(NormalBuffer);
	free(VertexBuffer);
}