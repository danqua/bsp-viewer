static void MaterialReset(material *Material)
{
    *Material = {};
}

static void MaterialBind(material Material)
{
    glUseProgram(Material.Program);
    s32 TextureSlot = 0;

    for (s32 UniformIndex = 0; UniformIndex < Material.NumUniforms; UniformIndex++)
    {
        uniform Uniform = Material.Uniforms[UniformIndex];
        switch (Uniform.Type)
        {
            case UNIFORM_TYPE_INT:
            {
                glUniform1i(Uniform.Location, Uniform.Value.Int);
            } break;

            case UNIFORM_TYPE_FLOAT:
            {
                glUniform1f(Uniform.Location, Uniform.Value.Float);
            } break;

            case UNIFORM_TYPE_VEC2:
            {
                glUniform2fv(Uniform.Location, 1, &Uniform.Value.Vec2[0]);

            } break;

            case UNIFORM_TYPE_VEC3:
            {
                glUniform3fv(Uniform.Location, 1, &Uniform.Value.Vec3[0]);
            } break;

            case UNIFORM_TYPE_VEC4:
            {
                glUniform4fv(Uniform.Location, 1, &Uniform.Value.Vec4[0]);
            } break;

            case UNIFORM_TYPE_SAMPLER_2D:
            {
                glUniform1i(Uniform.Location, TextureSlot);
                glActiveTexture(GL_TEXTURE0 + TextureSlot);
                glBindTexture(GL_TEXTURE_2D, Uniform.Value.Int);
                TextureSlot++;
            } break;
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, Material.Wireframe ? GL_LINE : GL_FILL);
    (Material.DepthTest ? glEnable : glDisable)(GL_DEPTH_TEST);
    (Material.CullFace ? glEnable : glDisable)(GL_CULL_FACE);
    (Material.Blending ? glEnable : glDisable)(GL_BLEND);
}

static uniform *FindUniform(material *Material, s32 Location)
{
    for (s32 UniformIndex = 0; UniformIndex < Material->NumUniforms; UniformIndex++)
    {
        if (Material->Uniforms[UniformIndex].Location == Location)
        {
            return (Material->Uniforms + UniformIndex);
        }
    }
    return 0;
}

static void SetUniformValue(material *Material, const char *Name, uniform_type Type, uniform_value Value)
{
    s32 Location = glGetUniformLocation(Material->Program, Name);

    if (Location != -1)
    {
        uniform *Uniform = FindUniform(Material, Location);
        if (!Uniform)
        {
            Uniform = &Material->Uniforms[Material->NumUniforms];
            Material->NumUniforms++;
        }
        Uniform->Type = Type;
        Uniform->Location = Location;
        Uniform->Value = Value;
    }
}

static void MaterialSetInt(material* Material, const char* Name, s32 Value)
{
    uniform_value UniformValue;
    UniformValue.Int = Value;
    SetUniformValue(Material, Name, UNIFORM_TYPE_INT, UniformValue);
}

static void MaterialSetFloat(material* Material, const char* Name, f32 Value)
{
    uniform_value UniformValue;
    UniformValue.Float = Value;
    SetUniformValue(Material, Name, UNIFORM_TYPE_FLOAT, UniformValue);
}

static void MaterialSetVec2(material* Material, const char* Name, v2 Value)
{
    uniform_value UniformValue;
    UniformValue.Vec2 = Value;
    SetUniformValue(Material, Name, UNIFORM_TYPE_VEC2, UniformValue);
}

static void MaterialSetVec3(material* Material, const char* Name, v3 Value)
{
    uniform_value UniformValue;
    UniformValue.Vec3 = Value;
    SetUniformValue(Material, Name, UNIFORM_TYPE_VEC3, UniformValue);
}

static void MaterialSetVec4(material* Material, const char* Name, v4 Value)
{
    uniform_value UniformValue;
    UniformValue.Vec4 = Value;
    SetUniformValue(Material, Name, UNIFORM_TYPE_VEC4, UniformValue);
}

static void MaterialSetTexture(material *Material, const char *Name, u32 Texture)
{
    uniform_value UniformValue = {};
    UniformValue.Int = Texture;
    SetUniformValue(Material, Name, UNIFORM_TYPE_SAMPLER_2D, UniformValue);
}