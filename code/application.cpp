static bool CreateWindowAndOpenGLContext(application* Application, s32 WindowWidth, s32 WindowHeight)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;

    Application->Window = SDL_CreateWindow(
        "BSPViewer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WindowWidth,
        WindowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (!Application->Window) return false;

    Application->Context = SDL_GL_CreateContext(Application->Window);
    if (!Application->Context) return false;

    SDL_GL_MakeCurrent(Application->Window, Application->Context);
    if (!gladLoadGL()) return false;

    SDL_SetRelativeMouseMode(SDL_TRUE);

    Application->IsRunning = true;
    memset(Application->Keys, 0, SDL_NUM_SCANCODES * sizeof(bool));
    return true;
}

inline application* CreateApplication()
{
    application* Application = (application*)malloc(sizeof *Application);
    if (CreateWindowAndOpenGLContext(Application, 800, 600)) {
        return Application;
    }
    free(Application);
    return nullptr;
}

inline void PollEvents(application* Application)
{
    SDL_Event Event = {};

    Application->RelMouse = v2(0, 0);

    while (SDL_PollEvent(&Event))
    {
        if (Event.type == SDL_QUIT)
        {
            Application->IsRunning = false;
            return;
        }
        else if (Event.type == SDL_KEYDOWN)
        {
            Application->Keys[Event.key.keysym.scancode] = true;
        }
        else if (Event.type == SDL_KEYUP)
        {
            Application->Keys[Event.key.keysym.scancode] = false;
        }
        else if (Event.type == SDL_MOUSEMOTION)
        {
            Application->Mouse.x = (float)Event.motion.x;
            Application->Mouse.y = (float)Event.motion.y;
            Application->RelMouse.x = (float)Event.motion.xrel;
            Application->RelMouse.y = (float)Event.motion.yrel;
        }
    }
}