#pragma once

struct program
{
    GLuint ID;

    // Attributes
    GLint VertPositionID;
    GLint VertUVID;

    // Uniforms
    GLint ProjectionMatrixID;
    GLint ModelViewMatrixID;
};

struct program_quad : program
{

};

struct program_quake : program
{
    // Attributes
    GLint VertLightMapID;

    // Uniforms
    GLint TextureID;
    GLint ColorPaletteID;
    GLint LightmapID;
    GLint BaseLightID;
};

struct render_info
{
    GLuint VAO;
    GLuint PositionVBO;
    GLuint UVVBO;
    GLuint LightMapVBO;
};

struct color
{
    u8 R;
    u8 G;
    u8 B;
};