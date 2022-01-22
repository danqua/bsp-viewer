static GLuint OpenGLCreateProgram(const char* VertexCode, const char* FragmentCode, program* Result)
{
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexCode, 0);
	glCompileShader(VertexShader);

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentCode, 0);
	glCompileShader(FragmentShader);

	GLuint Program = glCreateProgram();
	glAttachShader(Program, VertexShader);
	glAttachShader(Program, FragmentShader);
	glLinkProgram(Program);

	glValidateProgram(Program);
	GLint Linked = GL_FALSE;
	glGetProgramiv(Program, GL_LINK_STATUS, &Linked);

	if (!Linked)
	{
		GLsizei Ignored;
		char VertexErrors[2048];
		char FragmentErrors[2048];
		char ProgramErrors[2048];
		glGetShaderInfoLog(VertexShader, sizeof(VertexErrors), &Ignored, VertexErrors);
		glGetShaderInfoLog(FragmentShader, sizeof(FragmentErrors), &Ignored, FragmentErrors);
		glGetProgramInfoLog(Program, sizeof(ProgramErrors), &Ignored, ProgramErrors);

		Assert(!"Shader validation failed");
	}

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	Result->ID = Program;
	Result->VertPositionID = glGetAttribLocation(Program, "VertPosition");
	Result->VertNormalID = glGetAttribLocation(Program, "VertNormal");
	Result->VertUVID = glGetAttribLocation(Program, "VertUV");

	return Program;
}

static void CompileFlatShadedProgram(program_flat_shaded* Result)
{
	const char* VertexShaderCode = R"(
	#version 330 core
	layout (location = 0) in vec3 VertPosition;
	layout (location = 1) in vec3 VertNormal;
	layout (location = 2) in vec2 VertUV;

	out vec3 Normal;
	out vec2 UV;

	uniform mat4 ProjectionMatrix;
	uniform mat4 ModelViewMatrix;

	void main() {
		Normal = abs(VertNormal);
		UV = VertUV;
		gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(VertPosition, 1.0);
	}
	)";

	const char* FragmentShaderCode = R"(
	#version 330 core
	in vec3 Normal;
	in vec2 UV;
	out vec4 FragColor;
	uniform vec3 Color;
	uniform sampler2D Texture;
	void main() {
		FragColor = texture(Texture, UV);
	}
	)";

	GLuint Program = OpenGLCreateProgram(VertexShaderCode, FragmentShaderCode, Result);
	Result->ProjectionMatrixID = glGetUniformLocation(Program, "ProjectionMatrix");
	Result->ModelViewMatrixID = glGetUniformLocation(Program, "ModelViewMatrix");
}
