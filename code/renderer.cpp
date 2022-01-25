static GLuint OpenGLCreateTexture(const void* Pixels, s32 Width, s32 Height, GLenum Format)
{
    GLuint Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, GL_NONE, Format, GL_UNSIGNED_BYTE, Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    return Texture;
}

static void OpenGLTextureFilter(GLuint Texture, GLenum Filter)
{
    glBindTexture(GL_TEXTURE_2D, Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static GLuint OpenGLCreateProgram(const char* VertexCode, const char* FragmentCode, program* Result)
{
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexCode, 0);
    glCompileShader(VertexShader);

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentCode, 0);
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

        Assert(!"Shader validation failed");
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    Result->ID = Program;
    Result->VertPositionID = glGetAttribLocation(Program, "VertPosition");
    Result->VertUVID = glGetAttribLocation(Program, "VertUV");

    return Program;
}

static void CompileQuakeProgram(program_quake* Result)
{
    const char* VertexShaderCode = R"(
    #version 330 core
    layout (location = 0) in vec3 VertPosition;
    layout (location = 1) in vec2 VertUV;
    layout (location = 2) in vec2 VertLightMap;

    out vec2 UV;
    out vec2 LightMapUV;

    uniform mat4 ProjectionMatrix;
    uniform mat4 ModelViewMatrix;

    void main() {
        UV = VertUV;
        LightMapUV = VertLightMap;
        gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(VertPosition, 1.0);
    }
    )";

    const char* FragmentShaderCode = R"(
    #version 330 core
    in vec2 UV;
    in vec2 LightMapUV;
    out vec4 FragColor;
    uniform sampler2D Texture;
    uniform sampler2D ColorPalette;
    uniform sampler2D LightMapTexture;


    void main() {
        
        int Index = int(abs(texture(Texture, UV).r * 255));
        float S = (Index % 16) / 16.0;
        float T = (Index / 16) / 16.0;

        vec4 Diffuse = texture(ColorPalette, vec2(S, T));
        vec4 Light = vec4(vec3(texture(LightMapTexture, LightMapUV).r), 1.0);
        vec4 Color = Diffuse * Light * 2.0;
        FragColor = Color;
    }
    )";

    GLuint Program = OpenGLCreateProgram(VertexShaderCode, FragmentShaderCode, Result);
    Result->VertLightMapID = glGetAttribLocation(Program, "VertLightMap");
    Result->ProjectionMatrixID = glGetUniformLocation(Program, "ProjectionMatrix");
    Result->ModelViewMatrixID = glGetUniformLocation(Program, "ModelViewMatrix");
    Result->TextureID = glGetUniformLocation(Program, "Texture");
    Result->ColorPaletteID = glGetUniformLocation(Program, "ColorPalette");
    Result->LightmapID = glGetUniformLocation(Program, "LightMapTexture");
}

static void CompileQuadProgram(program_quad* Result)
{
    const char* VertexShaderCode = R"(
    #version 330 core
    layout (location = 0) in vec3 VertPosition;
    layout (location = 1) in vec2 VertUV;

    out vec2 UV;

    uniform mat4 ProjectionMatrix;
    uniform mat4 ModelViewMatrix;

    void main() {
        UV = VertUV;
        gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(VertPosition, 1.0);
    }
    )";

    const char* FragmentShaderCode = R"(
    #version 330 core
    in vec2 UV;
    out vec4 FragColor;
    uniform sampler2D Texture;
    void main() {
        FragColor = texture(Texture, UV).rrrr;
    }
    )";

    GLuint Program = OpenGLCreateProgram(VertexShaderCode, FragmentShaderCode, Result);
    Result->ProjectionMatrixID = glGetUniformLocation(Program, "ProjectionMatrix");
    Result->ModelViewMatrixID = glGetUniformLocation(Program, "ModelViewMatrix");
}