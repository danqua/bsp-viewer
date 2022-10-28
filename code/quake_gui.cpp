static gui_state GuiState;

static bitmap_font LoadBitmapFont(const char *Filename, s32 CharacterWidth, s32 CharacterHeight)
{
    bitmap_font Result;
    Result.CharacterWidth = CharacterWidth;
    Result.CharacterHeight = CharacterHeight;
    Result.Surface = SDL_LoadBMP(Filename);
    Result.CharactersPerRow = Result.Surface->w / Result.CharacterWidth;
    SDL_SetColorKey(Result.Surface, SDL_TRUE, SDL_MapRGB(Result.Surface->format, 255, 0, 255));
    return Result;    
}

static void GetBitmapFontCharacterOffset(bitmap_font Font, s32 CharIndex, s32 *OffsetX, s32 *OffsetY)
{
    *OffsetX = (CharIndex % Font.CharactersPerRow) * Font.CharacterWidth;
    *OffsetY = (CharIndex / Font.CharactersPerRow) * Font.CharacterHeight;
}

static void DrawText(bitmap_font Font, SDL_Surface *DstSurface, s32 X, s32 Y, const char *Text)
{
    const char *Character = Text;
    while (*Character)
    {
        s32 CharIndex = (s32)(*Character - ' ');
        SDL_Rect SrcRect;
        SrcRect.w = Font.CharacterWidth;
        SrcRect.h = Font.CharacterHeight;
        GetBitmapFontCharacterOffset(Font, CharIndex, &SrcRect.x, &SrcRect.y);

        SDL_Rect DstRect;
        DstRect.x = X;
        DstRect.y = Y;
        DstRect.w = Font.CharacterWidth * FONT_SCALE_FACTOR;
        DstRect.h = Font.CharacterHeight * FONT_SCALE_FACTOR;
        X = X + Font.CharacterWidth * FONT_SCALE_FACTOR;

        SDL_BlitScaled(Font.Surface, &SrcRect, DstSurface, &DstRect);
        Character++;
    } 
}

static void GuiInit()
{
    GuiState.Font = LoadBitmapFont("images/characters.bmp", 7, 9);

    GuiState.Canvas = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, VIDEO_WIDTH, VIDEO_HEIGHT, 32, SDL_PIXELFORMAT_RGBA8888);
    SDL_FillRect(GuiState.Canvas, 0, 0);

    vertex QuadVertices[] = {
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
    };

    u32 Indices[] = { 0, 2, 1, 0, 3, 2 };
    s32 NumIndices = 6;
        
    u32 Texture = CreateTexture(0, VIDEO_WIDTH, VIDEO_HEIGHT, GL_RGBA, GL_NEAREST, GL_CLAMP_TO_EDGE, 0);
    v3 FontColor = v3(0.0f, 1.0f, 1.0f);
    GuiState.Material.Program = GetShader("GuiShader");
    GuiState.Material.Blending = true;
    MaterialSetVec3(&GuiState.Material, "Color", FontColor);
    MaterialSetTexture(&GuiState.Material, "Texture", Texture);
    GuiState.TextureTarget = Texture;
    GuiState.Mesh = CreateMesh(QuadVertices, 4, Indices, NumIndices);
}

static void GuiBegin()
{
    SDL_FillRect(GuiState.Canvas, 0, 0);
}

static void GuiDrawText(s32 X, s32 Y, const char *Format, ...)
{
    va_list Args;
    va_start(Args, Format);
    char Buffer[256] = {};
    vsprintf(Buffer, Format, Args);
    va_end(Args);
    DrawText(GuiState.Font, GuiState.Canvas, X, Y, Buffer);
}

static void GuiEnd()
{
    UpdateTexture(GuiState.TextureTarget, 0, 0, VIDEO_WIDTH, VIDEO_HEIGHT, GL_RGBA, GuiState.Canvas->pixels);
    MaterialBind(GuiState.Material);
    MeshDraw(GuiState.Mesh);
}