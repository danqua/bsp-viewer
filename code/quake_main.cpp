#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <vector>
#include <time.h>

#include "quake_types.h"
#include "quake_renderer.h"
#include "quake_gui.h"
#include "quake_bsp_info.h"
#include "quake_map.h"

#define VIDEO_WIDTH 1920
#define VIDEO_HEIGHT 1080

struct input
{
    s32 MousePositionX;
    s32 MousePositionY;
    s32 MousePositionXRel;
    s32 MousePositionYRel;
    s32 MouseButton;
    s32 Keyboard[SDL_NUM_SCANCODES];
};

static SDL_Window* Window;
static SDL_GLContext Context;
static camera Camera;

#include "quake_string_utils.cpp"
#include "quake_camera.cpp"
#include "quake_renderer.cpp"
#include "quake_shader.cpp"
#include "quake_material.cpp"
#include "quake_gui.cpp"
#include "quake_map.cpp"

/*
MS: 3
FPS: ~270
DRAW CALLS: 6500
*/

int SDL_main(int ArgCount, char **ArgValues)
{
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    Window = SDL_CreateWindow(
        "Harm",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        VIDEO_WIDTH, VIDEO_HEIGHT,
        SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    
    Context = SDL_GL_CreateContext(Window);
    gladLoadGL();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowMouseGrab(Window, SDL_TRUE);
    SDL_ShowWindow(Window);
    SDL_GL_SetSwapInterval(0);

    // Init resources
    LoadShader("shaders/surface.glsl", "SurfaceShader");
    LoadShader("shaders/sky.glsl", "SkyShader");
    LoadShader("shaders/gui.glsl", "GuiShader");
    LoadShader("shaders/debug.glsl", "DebugShader");

    GuiInit();

    input Input = {};

    f32 AspectRatio = (f32)VIDEO_WIDTH / VIDEO_HEIGHT;
    Camera = CreateCamera(90.0f, AspectRatio, 0.1f, 65536.0f);
    Camera.Speed = 320.0f;
    Camera.Position.x = 535.0f;
    Camera.Position.y = 86.0f;
    Camera.Position.z = -256.0f;
    Camera.Rotation.y = 180.0f;

    LoadMap(ArgValues[1]);
        
    u32 Indices[128] = {};
    u64 OldTime = SDL_GetPerformanceCounter();
    f32 Time = 0.0f;

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    
    b32 Running = 1;
    while (Running)
    {
        SDL_Event event;

        Input.MousePositionXRel = 0;
        Input.MousePositionYRel = 0;
        
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                Running = false;
                break;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    Running = false;
                    break;
                }
                Input.Keyboard[event.key.keysym.scancode] = 1;
            }
            else if (event.type == SDL_KEYUP)
            {
                Input.Keyboard[event.key.keysym.scancode] = 0;

                if (event.key.keysym.scancode == SDL_SCANCODE_F5)
                {
                }
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                Input.MousePositionX = event.motion.x;
                Input.MousePositionY = event.motion.y;
                Input.MousePositionXRel = event.motion.xrel;
                Input.MousePositionYRel = event.motion.yrel;
            }
        }

        u64 Elapsed = SDL_GetPerformanceCounter() - OldTime;
        f32 DeltaTime = (f32)Elapsed / SDL_GetPerformanceFrequency();
        u32 FramesPerSecond = (s32)(1.0f / DeltaTime);
        OldTime = SDL_GetPerformanceCounter();
        Time += DeltaTime;

        CameraHandleUserInput(&Camera, &Input, DeltaTime);

        GuiBegin();

        GuiDrawText(8, 8,  "MS %3d", (s32)(DeltaTime * 1000.0f));
        GuiDrawText(8, 32, "FPS %d", (s32)(1.0f / DeltaTime));

        u64 FrameTime = SDL_GetPerformanceCounter();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
        DrawMap(Time);

        s32 FrameTimeDelta = SDL_GetPerformanceCounter() - FrameTime;
        f32 RenderTimeMS = ((f32)FrameTimeDelta / SDL_GetPerformanceFrequency()) * 1000.0f;

        GuiEnd();

        SDL_GL_SwapWindow(Window);
    }

    SDL_DestroyWindow(Window);
    SDL_GL_DeleteContext(Context);
    SDL_Quit();
    return 0;
}