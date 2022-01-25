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

int SDL_main(int Argc, char** Argv)
{
    
    application* Application = CreateApplication();

    program_quake FlatShadedProgram;
    program_quad QuadProgram;

    CreateColorPalette();

    camera Camera;
    CreateCamera(&Camera, 60.0f, 16.0f / 9.0f, 0.1f, 65535.0f);
    Camera.Position = v3(544.0f, 288.0f, 32.0f + 56.0f);

    map Map;
    bool Success = LoadMap(&Map, "e1m1.bsp");
    if (!Success) return -1;

    CreateVertexBuffer(&Map);

    m4x4 Model = m4x4(1.0f);

    CompileQuakeProgram(&FlatShadedProgram);
    CompileQuadProgram(&QuadProgram);

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

        glUseProgram(FlatShadedProgram.ID);

        glBindVertexArray(Map.RenderInfo.VAO);

        glUniformMatrix4fv(FlatShadedProgram.ProjectionMatrixID, 1, GL_FALSE, &Camera.Projection[0][0]);
        glUniformMatrix4fv(FlatShadedProgram.ModelViewMatrixID, 1, GL_FALSE, &(Camera.View * Model)[0][0]);

        GLint Offset = 0;
        bsp_face* Face = Map.Faces;
        while (Face != &Map.Faces[Map.FaceCount])
        {

            glUniform1i(FlatShadedProgram.TextureID, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Textures[Map.TexInfos[Face->TexInfo].Miptex].ID);

            glUniform1i(FlatShadedProgram.ColorPaletteID, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, ColorPalette);


            glUniform1i(FlatShadedProgram.LightmapID, 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, LightMapTextureID);

            glDrawArrays(GL_TRIANGLE_FAN, Offset, Face->EdgeCount);
            Offset += Face->EdgeCount;
            Face++;
        }

        SDL_GL_SwapWindow(Application->Window);
    }
    
    return 0;
}