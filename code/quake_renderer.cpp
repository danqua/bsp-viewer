static bool PointInsideViewFrustum(v3 Point, const m4x4& ModelViewProjection)
{
    v4 PointNDC = ModelViewProjection * v4(Point, 1.0f);
    PointNDC = PointNDC / PointNDC.w;
    for (s32 Component = 0; Component < 3; Component++)
    {
        if (PointNDC[Component] > 1.0f)
        {
            return false;
        }
    }
    return true;
}

static b32 AABBInsideViewFrustum(aabb AABB, const m4x4& ModelViewProjection)
{
    v3 A = AABB.Min + v3(0.0f, 0.0f, 0.0f);
    v3 B = AABB.Min + v3(AABB.Max.x, 0.0f, 0.0f);
    v3 C = AABB.Min + v3(AABB.Max.x, AABB.Max.y, 0.0f);
    v3 D = AABB.Min + v3(0.0f, AABB.Max.y, 0.0f);
    v3 E = AABB.Min + v3(0.0f, 0.0f, AABB.Max.z);
    v3 F = AABB.Min + v3(AABB.Max.x, 0.0f, AABB.Max.z);
    v3 G = AABB.Min + v3(AABB.Max.x, AABB.Max.y, AABB.Max.z);
    v3 H = AABB.Min + v3(0.0f, AABB.Max.y, AABB.Max.z);

    if (PointInsideViewFrustum(A, ModelViewProjection) ||
        PointInsideViewFrustum(B, ModelViewProjection) ||
        PointInsideViewFrustum(C, ModelViewProjection) ||
        PointInsideViewFrustum(D, ModelViewProjection) ||
        PointInsideViewFrustum(E, ModelViewProjection) ||
        PointInsideViewFrustum(F, ModelViewProjection) ||
        PointInsideViewFrustum(G, ModelViewProjection) ||
        PointInsideViewFrustum(H, ModelViewProjection))
        return 1;
    return 0;
}

static mesh CreateMesh(const vertex *Vertices, s32 NumVertices, const u32 *IndexData, s32 NumIndices)
{
    mesh Mesh = {};
    Mesh.NumVertices = NumVertices;
    Mesh.Topology = GL_TRIANGLE_FAN;

    glGenVertexArrays(1, &Mesh.VAO);
    glBindVertexArray(Mesh.VAO);
    glGenBuffers(1, &Mesh.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * NumVertices, Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, Position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, TexCoord));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, Lightmap));

    if (NumIndices > 0)
    {
        glGenBuffers(1, &Mesh.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * NumIndices, IndexData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        Mesh.NumIndices = NumIndices;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    Mesh.BoundingBox.Min = v3(FLT_MAX);
    Mesh.BoundingBox.Max = v3(-FLT_MAX);

    for (s32 VertexIndex = 0; VertexIndex < NumVertices; VertexIndex++)
    {
        for (s32 ComponentIndex = 0; ComponentIndex < 3; ComponentIndex++)
        {
            if (Mesh.BoundingBox.Min[ComponentIndex] > Vertices[VertexIndex].Position[ComponentIndex])
                Mesh.BoundingBox.Min[ComponentIndex] = Vertices[VertexIndex].Position[ComponentIndex];
            if (Mesh.BoundingBox.Max[ComponentIndex] < Vertices[VertexIndex].Position[ComponentIndex])
                Mesh.BoundingBox.Max[ComponentIndex] = Vertices[VertexIndex].Position[ComponentIndex];
        }
    }

    return Mesh;
}

static GLuint CreateTexture(const void *Data, s32 Width, s32 Height, GLenum Format, GLenum Filter, GLenum WrapMode, b32 GenerateMipmap = 0)
{
    GLuint Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, GL_NONE, Format, GL_UNSIGNED_BYTE, Data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapMode);
    if (GenerateMipmap)
    {
        if (Filter != GL_NEAREST)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return Texture;
}

static void UpdateTexture(u32 Texture, s32 XOffset, s32 YOffset, s32 Width, s32 Height, s32 Format, const void *Pixels)
{
    glBindTexture(GL_TEXTURE_2D, Texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, XOffset, YOffset, Width, Height, Format, GL_UNSIGNED_BYTE, Pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static GLuint OpenGLCreateShaderProgram(const char *VertexShaderCode, const char *FragmentShaderCode)
{
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderCode, 0);
    glCompileShader(VertexShader);

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderCode, 0);
    glCompileShader(FragmentShader);

    GLuint Program = glCreateProgram();
    glAttachShader(Program, VertexShader);
    glAttachShader(Program, FragmentShader);
    glLinkProgram(Program);

    glValidateProgram(Program);
    GLint Linked = GL_FALSE;
    glGetProgramiv(Program, GL_LINK_STATUS, &Linked);

    if (!Linked)
    {
        GLsizei Ignored;
        char VertexErrors[2048];
        char FragmentErrors[2048];
        char ProgramErrors[2048];
        glGetShaderInfoLog(VertexShader, sizeof(VertexErrors), &Ignored, VertexErrors);
        glGetShaderInfoLog(FragmentShader, sizeof(FragmentErrors), &Ignored, FragmentErrors);
        glGetProgramInfoLog(Program, sizeof(ProgramErrors), &Ignored, ProgramErrors);
        
        SDL_Log("[Vertex Shader]\n");
        SDL_Log("%s\n", VertexErrors);
        SDL_Log("[Fragment Shader]\n");
        SDL_Log("%s\n", FragmentErrors);
        SDL_Log("[Program Errors]\n");
        SDL_Log("%s\n", ProgramErrors);
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    return Program;
}

static void MeshDraw(mesh Mesh)
{
    glBindVertexArray(Mesh.VAO);
    if (Mesh.NumIndices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh.EBO);
        glDrawElements(Mesh.Topology, Mesh.NumIndices, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(Mesh.Topology, 0, Mesh.NumVertices);
    }
}

static void MeshDrawIndexed(mesh Mesh, s32 NumIndices, u64 Offset)
{
    glBindVertexArray(Mesh.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Mesh.EBO);
    glDrawElements(Mesh.Topology, NumIndices, GL_UNSIGNED_INT, (const void *)Offset);
}