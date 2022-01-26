#define _CRT_SECURE_NO_WARNINGS

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "types.h"
#include "application.h"
#include "renderer.h"
#include "camera.h"
#include "bsp.h"
#include "map.h"

#include "application.cpp"
#include "renderer.cpp"
#include "camera.cpp"
#include "map.cpp"

program_quake QuakeProgram;

void DrawFaces(s32 FirstFace, s32 NumFaces)
{
    surface* Surface = &Surfaces[FirstFace];

    while (Surface != (&Surfaces[FirstFace] + NumFaces))
    {
        if (Surface->Texture)
        {
            glUseProgram(QuakeProgram.ID);
            glUniform1i(QuakeProgram.TextureID, 0);
            glUniform1i(QuakeProgram.BaseLightID, Surface->Styles[0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Surface->Texture->ID);

            glDrawArrays(GL_TRIANGLE_FAN, Surface->VBOOffset, Surface->NumEdges);
        }
        Surface++;
    }
}

void DrawNode(s32 NodeNum)
{
    if ((NodeNum & 0x8000) == 0)
    {
        bsp_node* Node = &Map.Nodes[NodeNum];

        DrawFaces(Node->FirstFace, Node->FaceCount);

        DrawNode(Node->Children[0]);
        DrawNode(Node->Children[1]);
    }
}

void DrawMap(camera& Camera)
{
    bsp_model* Model = Map.Models;

    // Bind the color palette texture
    glUniform1i(QuakeProgram.ColorPaletteID, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GameState.ColorPaletteTID);

    // Bind the single lightmap texture
    glUniform1i(QuakeProgram.LightmapID, 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, GameState.LightmapTID);

    if (PointInBounds(Model->Min, Model->Max, Camera.Position))
    {
        bsp_node* Node = &Map.Nodes[Model->HeadNodes[0]];
        DrawNode(Node->Children[0]);
        DrawNode(Node->Children[1]);
    }
}

int SDL_main(int Argc, char** Argv)
{
    
    application* Application = CreateApplication();

    camera Camera;
    CreateCamera(&Camera, 75.0f, 16.0f / 9.0f, 0.1f, 65535.0f);
    Camera.Position = v3(544.0f, 288.0f, 32.0f + 56.0f);

    LoadMap("start.bsp");
    CreateColorPalette();
    CreateSurfaces();
    CreateVertexBuffer();

    CompileQuakeProgram(&QuakeProgram);

    u64 OldTime = SDL_GetPerformanceCounter();
    u64 ElapsedTime = 0;
    f32 DeltaTime = 0.0f;


    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while (Application->IsRunning)
    {
        ElapsedTime = SDL_GetPerformanceCounter() - OldTime;
        OldTime = SDL_GetPerformanceCounter();
        DeltaTime = (float)ElapsedTime / SDL_GetPerformanceFrequency();

        PollEvents(Application);

        UpdateCameraMovement(&Camera, Application, DeltaTime);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(QuakeProgram.ID);

        glBindVertexArray(GameState.RenderGroup.VAO);

        glUniformMatrix4fv(QuakeProgram.ProjectionMatrixID, 1, GL_FALSE, &Camera.Projection[0][0]);
        glUniformMatrix4fv(QuakeProgram.ModelViewMatrixID, 1, GL_FALSE, &(Camera.View)[0][0]);

        DrawMap(Camera);

        SDL_GL_SwapWindow(Application->Window);
    }
    
    return 0;
}