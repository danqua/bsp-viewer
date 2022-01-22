#pragma once

struct program
{
	GLuint ID;

	// Attributes
	GLint VertPositionID;
	GLint VertNormalID;
	GLint VertUVID;

	GLint ProjectionMatrixID;
	GLint ModelViewMatrixID;
};

struct program_flat_shaded : program
{
};

struct render_info
{
	GLuint VAO;
	GLuint PositionVBO;
	GLuint NormalVBO;
	GLuint UVVBO;
};