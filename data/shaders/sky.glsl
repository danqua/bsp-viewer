#vertex
#version 460 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;
layout (location = 2) in vec2 Lightmap;

layout (location = 0) uniform mat4 ProjectionMatrix;
layout (location = 1) uniform mat4 ViewMatrix;
layout (location = 2) uniform mat4 ModelMatrix;
layout (location = 3) uniform vec3 CameraPosition;

out vec3 SkyTexCoord;

void main()
{
    SkyTexCoord = mat3(ModelMatrix) * Position - CameraPosition;
    mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.0);
}

#fragment
#version 460 core
out vec4 FragColor;

in vec3 SkyTexCoord;

uniform float Time;
uniform sampler2D Texture0;
uniform sampler2D Texture2;

void main()
{
    float Scroll = Time / 8.0;
    float StretchFactor = 4.0;
    vec3 SkyDir = SkyTexCoord;
    SkyDir.y *= StretchFactor;
    SkyDir = normalize(SkyDir) * StretchFactor;
    vec2 SkyUV = vec2(SkyDir.x + Scroll, SkyDir.z - Scroll);
    FragColor = texture(Texture0, SkyUV);
    if ((FragColor.r + FragColor.g + FragColor.b) < 0.1)
    {
        Scroll = Time / 16.0;
        SkyUV = vec2(SkyDir.x + Scroll, SkyDir.z - Scroll);
        FragColor = texture(Texture2, SkyUV);
    }
}