static const v3 GlobalWorldRightVector   = v3(1.0f, 0.0f, 0.0f);
static const v3 GlobalWorldUpVector      = v3(0.0f, 1.0f, 0.0f);
static const v3 GlobalWorldForwardVector = v3(0.0f, 0.0f, 1.0f);

static v3 GetForwardVector(v3 Rotation)
{
    v3 RadiansRotation = glm::radians(Rotation);
    v3 Result = glm::quat(RadiansRotation) * GlobalWorldForwardVector;
    return Result;
}

static v3 GetRightVector(v3 Rotation)
{
    v3 RadiansRotation = glm::radians(Rotation);
    v3 Result = glm::quat(RadiansRotation) * GlobalWorldRightVector;
    return Result;
}

static camera CreateCamera(f32 Fov, f32 Aspect, f32 Near, f32 Far)
{
    camera Result = {};
    Result.Fov = Fov;
    Result.Aspect = Aspect;
    Result.Near = Near;
    Result.Far = Far;
    Result.ProjectionMatrix = glm::perspective(glm::radians(Fov), Aspect, Near, Far);
    return Result;
}

static m4x4 CameraGetViewMatrix(camera *Camera)
{
    v3 Target = Camera->Position + GetForwardVector(Camera->Rotation);
    m4x4 Result = glm::lookAt(Camera->Position, Target, GlobalWorldUpVector);
    return Result;
}

static void CameraHandleUserInput(camera *Camera, input *Input, f32 DeltaTime)
{
    f32 Speed = Camera->Speed;
    f32 MouseSensitivity = 70.0f;
    if (Input->Keyboard[SDL_SCANCODE_LSHIFT]) Speed *= 2.0f;
    if (Input->Keyboard[SDL_SCANCODE_W]) Camera->Position += GetForwardVector(Camera->Rotation) * Speed * DeltaTime;
    if (Input->Keyboard[SDL_SCANCODE_S]) Camera->Position -= GetForwardVector(Camera->Rotation) * Speed * DeltaTime;
    if (Input->Keyboard[SDL_SCANCODE_A]) Camera->Position += GetRightVector(Camera->Rotation) * Speed * DeltaTime;
    if (Input->Keyboard[SDL_SCANCODE_D]) Camera->Position -= GetRightVector(Camera->Rotation) * Speed * DeltaTime;
    if (Input->Keyboard[SDL_SCANCODE_E]) Camera->Position += GlobalWorldUpVector * Speed * DeltaTime;
    if (Input->Keyboard[SDL_SCANCODE_Q]) Camera->Position -= GlobalWorldUpVector * Speed * DeltaTime;
    Camera->Rotation.x += (float)Input->MousePositionYRel * MouseSensitivity * DeltaTime;
    Camera->Rotation.y -= (float)Input->MousePositionXRel * MouseSensitivity * DeltaTime;

    Camera->Rotation.x = glm::clamp(Camera->Rotation.x, -89.0f, 89.0f);
}