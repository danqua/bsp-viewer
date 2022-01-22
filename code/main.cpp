#define _CRT_SECURE_NO_WARNINGS

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "types.h"
#include "application.h"
#include "renderer.h"
#include "draw.h"
#include "camera.h"
#include "bsp.h"
#include "map.h"

#include "application.cpp"
#include "renderer.cpp"
#include "camera.cpp"
#include "map.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int SDL_main(int Argc, char** Argv)
{
    
    application* Application = CreateApplication();

    program_flat_shaded FlatShadedProgram;

    camera Camera;
    CreateCamera(&Camera, 60.0f, 4.0f / 3.0f, 0.1f, 65535.0f);
    Camera.Position = v3(544.0f, 288.0f, 32.0f + 56.0f);


    map Map;
    bool Success = LoadMap(&Map, "start.bsp");
    if (!Success) return -1;

    CreateVertexBuffer(&Map);

    m4x4 Model = m4x4(1.0f);

    CompileFlatShadedProgram(&FlatShadedProgram);

    u64 OldTime = SDL_GetPerformanceCounter();
    u64 ElapsedTime = 0;
    f32 DeltaTime = 0.0f;


    s32 Width, Height;
    u8* Pixels = stbi_load("dummy.png", &Width, &Height, 0, STBI_rgb);
    GLuint TextureID;
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, GL_NONE, GL_RGB, GL_UNSIGNED_BYTE, Pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(Pixels);

    while (Application->IsRunning)
    {
        ElapsedTime = SDL_GetPerformanceCounter() - OldTime;
        OldTime = SDL_GetPerformanceCounter();
        DeltaTime = (float)ElapsedTime / SDL_GetPerformanceFrequency();

        PollEvents(Application);

        UpdateCameraMovement(&Camera, Application, DeltaTime);


        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(FlatShadedProgram.ID);
        
        glBindVertexArray(Map.RenderInfo.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, Map.RenderInfo.PositionVBO);
        glEnableVertexAttribArray(FlatShadedProgram.VertPositionID);
        glVertexAttribPointer(FlatShadedProgram.VertPositionID, 3, GL_FLOAT, GL_FALSE, sizeof(v3), 0);

        glBindBuffer(GL_ARRAY_BUFFER, Map.RenderInfo.NormalVBO);
        glEnableVertexAttribArray(FlatShadedProgram.VertNormalID);
        glVertexAttribPointer(FlatShadedProgram.VertNormalID, 3, GL_FLOAT, GL_FALSE, sizeof(v3), 0);

        glBindBuffer(GL_ARRAY_BUFFER, Map.RenderInfo.UVVBO);
        glEnableVertexAttribArray(FlatShadedProgram.VertUVID);
        glVertexAttribPointer(FlatShadedProgram.VertUVID, 2, GL_FLOAT, GL_FALSE, sizeof(v2), 0);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureID);

        glUniformMatrix4fv(FlatShadedProgram.ProjectionMatrixID, 1, GL_FALSE, &Camera.Projection[0][0]);
        glUniformMatrix4fv(FlatShadedProgram.ModelViewMatrixID, 1, GL_FALSE, &(Camera.View * Model)[0][0]);

        GLint Offset = 0;
        bsp_face* Face = Map.Faces;
        while (Face != &Map.Faces[Map.FaceCount])
        {
            glDrawArrays(GL_TRIANGLE_FAN, Offset, Face->EdgeCount);
            Offset += Face->EdgeCount;
            Face++;
        }
        SDL_GL_SwapWindow(Application->Window);
    }
    
    return 0;
}