#ifndef HARM_GUI_H
#define HARM_GUI_H

#define FONT_SCALE_FACTOR 2

struct bitmap_font
{
    s32 CharacterWidth;
    s32 CharacterHeight;
    s32 CharactersPerRow;
    SDL_Surface *Surface;
};

struct gui_state
{
    mesh Mesh;
    material Material;
    u32 TextureTarget;

    s32 ScaleFactor;
    bitmap_font Font;
    SDL_Surface *Canvas;
};

#endif