static v3 CameraForwardVector(camera* Camera)
{
    v3 Forward = glm::quat(Camera->Rotation) * v3(0, 1, 0);
    return Forward;
}

static v3 CameraRightVector(camera* Camera)
{
    v3 Right = glm::quat(Camera->Rotation) * v3(1, 0, 0);
    return Right;
}

static void CreateCamera(camera* Result, f32 Fov, f32 Aspect, f32 Near, f32 Far)
{
    v3 Position = v3(0, 0, 0);
    v3 Rotation = v3(0, 0, 0);
    v3 ForwardVector = v3(-1, 0, 0);
    
    Result->Position = Position;
    Result->Rotation = Rotation;
    Result->Speed = 320.0f;
    Result->Projection = glm::perspective(glm::radians(Fov), Aspect, Near, Far);
    Result->NeedsUpdate = true;
}

static void UpdateCameraMovement(camera* Camera, const application* Application, f32 DeltaTime)
{
    f32 SpeedMultiplier = 1.0f;

    // Handle user input
    if (Application->Keys[SDL_SCANCODE_LSHIFT])
    {
        SpeedMultiplier = 1.5f;
    }
    if (Application->Keys[SDL_SCANCODE_W])
    {
        Camera->Position += CameraForwardVector(Camera) * Camera->Speed * SpeedMultiplier * DeltaTime;
        Camera->NeedsUpdate = true;
    }
    if (Application->Keys[SDL_SCANCODE_S])
    {
        Camera->Position -= CameraForwardVector(Camera) * Camera->Speed * SpeedMultiplier * DeltaTime;
        Camera->NeedsUpdate = true;
    }
    if (Application->Keys[SDL_SCANCODE_A])
    {
        Camera->Position -= CameraRightVector(Camera) * Camera->Speed * SpeedMultiplier * DeltaTime;
        Camera->NeedsUpdate = true;
    }
    if (Application->Keys[SDL_SCANCODE_D])
    {
        Camera->Position += CameraRightVector(Camera) * Camera->Speed * SpeedMultiplier * DeltaTime;
        Camera->NeedsUpdate = true;
    }
    if (Application->Keys[SDL_SCANCODE_E])
    {
        Camera->Position.z += Camera->Speed * SpeedMultiplier * DeltaTime;
        Camera->NeedsUpdate = true;
    }
    if (Application->Keys[SDL_SCANCODE_Q])
    {
        Camera->Position.z -= Camera->Speed * SpeedMultiplier * DeltaTime;
        Camera->NeedsUpdate = true;
    }

    if (Application->RelMouse.x != 0.0f || Application->RelMouse.y != 0.0f)
    {
        Camera->Rotation.x -= Application->RelMouse.y * DeltaTime;
        Camera->Rotation.z -= Application->RelMouse.x * DeltaTime;
        Camera->Rotation.x = glm::clamp(Camera->Rotation.x, -glm::radians(89.0f), glm::radians(89.0f));
        Camera->NeedsUpdate = true;
    }

    // Update the camera
    if (Camera->NeedsUpdate)
    {
        v3 ForwardVector = CameraForwardVector(Camera);
        Camera->View = glm::lookAt(Camera->Position, Camera->Position + ForwardVector, v3(0, 0, 1));
        Camera->NeedsUpdate = false;
    }
}