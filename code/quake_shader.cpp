#define SHADER_STORAGE_MAX 32

struct shader_storage_entry
{
    char Name[32];
    u32 Program;
};

static s32 NumStorageEntries;
static shader_storage_entry ShaderStorage[SHADER_STORAGE_MAX];

static void AddShaderToStorage(const char* Name, u32 Program)
{
    if (NumStorageEntries < SHADER_STORAGE_MAX)
    {
        strcpy(ShaderStorage[NumStorageEntries].Name, Name);
        ShaderStorage[NumStorageEntries].Program = Program;
        NumStorageEntries++;
    }
}

static shader_storage_entry *FindShader(const char *Name)
{
    for (s32 EntryIndex = 0; EntryIndex < NumStorageEntries; EntryIndex++)
    {
        shader_storage_entry *Entry = ShaderStorage + EntryIndex;
        if (strcmp(Name, Entry->Name) == 0)
        {
            return Entry;
        }
    }
    return 0;
}

static u32 GetShader(const char *Name)
{
    shader_storage_entry *Entry = FindShader(Name);
    if (Entry)
    {
        return Entry->Program;
    }
    return 0;
}

static u32 LoadShader(const char *Filename, const char *Name)
{
    shader_storage_entry *Entry = FindShader(Name);

    if (Entry)
    {
        return Entry->Program;
    }

    FILE *Stream = fopen(Filename, "r");
    if (!Stream) return 0;
    const size_t LineBufferLength = 256;
    char LineBuffer[LineBufferLength];
    char VertexShaderCode[4096] = {};
    char FragmentShaderCode[4096] = {};
    char *ShaderCode = 0;

    while (fgets(LineBuffer, LineBufferLength, Stream))
    {
        RTrim(LineBuffer);
        if (strcmp(LineBuffer, "#vertex") == 0)
        {
            ShaderCode = VertexShaderCode;
        }
        else if (strcmp(LineBuffer, "#fragment") == 0)
        {
            ShaderCode = FragmentShaderCode;
        }
        else
        {
            if (strlen(LineBuffer))
            {
                strcat(ShaderCode, LineBuffer);
                strcat(ShaderCode, "\n");
            }
        }
    }

    fclose(Stream);

    u32 Program = OpenGLCreateShaderProgram(VertexShaderCode, FragmentShaderCode);
    AddShaderToStorage(Name, Program);
    return Program;
}