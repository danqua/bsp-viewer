#vertex
#version 460 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

out vec2 UV;

void main()
{
    gl_Position = vec4(Position, 1.0);
    UV = TexCoord;
}

#fragment
#version 460 core
in vec2 UV;
out vec4 FragColor;

uniform vec3 Color;
uniform sampler2D Texture;

void main()
{
    
    vec2 FlippedUV = vec2(UV.x, 1.0 - UV.y);
    FragColor = texture(Texture, FlippedUV);
    FragColor.rgb *= Color;
}