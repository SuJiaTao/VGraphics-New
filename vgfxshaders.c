
/* ========== <vgfxshaders.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Shader loading and creation logic for RenderThread		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfx.h"


/* ========== SHADER SOURCE						==========	*/
VGFXAPI vPCHAR vGFXGetVertexShaderSource(void)
{
	return
		"#version 460 core\n"
		"\n"
		"layout (location = 0) in vec2 v_position;\n"
		"layout (location = 1) uniform vec4 v_color;\n"
		"layout (location = 2) uniform mat4 v_projectionMatrix;\n"
		"layout (location = 3) uniform mat4 v_modelMatrix;\n"
		"layout (location = 4) uniform mat4 v_textureMatrix;\n"
		"\n"
		"out vec2 f_textureUV;\n"
		"out vec4 f_color;\n"
		"\n"
		"void main()\n"
		"{\n"
		"\t/* uv is reduced to range 0 - 1 */\n"
		"\tf_textureUV = (v_position / 2.0) + 0.5;\n"
		"\n"
		"\t/* multiply by texture matrix */\n"
		"\tvec4 tempUV4 = vec4(f_textureUV, 0.0, 1.0) * v_textureMatrix;\n"
		"\tf_textureUV = tempUV4.xy;\n"
		"\n"
		"\t/* fragment color is same as vertex color */\n"
		"\tf_color = v_color;\n"
		"\n"
		"\t/* set final vertex position */\t \n"
		"\tgl_Position = v_projectionMatrix * v_modelMatrix * vec4(v_position, 0.0, 1.0);\t\n"
		"}";
}

VGFXAPI vPCHAR vGFXGetFragmentShaderSource(void)
{
	return
		"#version 460\n"
		"\n"
		"in vec2 f_textureUV;\n"
		"in vec4 f_color;\n"
		"\n"
		"uniform sampler2D f_texture;\n"
		"\n"
		"out vec4 FragColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"\tFragColor = texture(f_texture, f_textureUV) * f_color;\n"
		"\n"
		"\t/* dithering alogrithm */\n"
		"\tif (FragColor.a <= 0.97)\n"
		"\t{\n"
		"\t\tif (FragColor.a <= 0.03) discard;\n"
		"\t\tint x = int(gl_FragCoord.x);\n"
		"\t\tint y = int(gl_FragCoord.y);\n"
		"\n"
		"\t\tif (FragColor.a < 0.50)\n"
		"\t\t{\n"
		"\t\t\tint stride = int(1.25 / FragColor.a);\n"
		"\t\t\tint phase  = (y % stride) * (stride >> 3);\n"
		"\t\t\tint step = x + (y * (stride >> 1)) + phase;\n"
		"\t\t\tif (step % stride != 0) discard;\n"
		"\t\t}\n"
		"\t\telse\n"
		"\t\t{\n"
		"\t\t\tint stride = int(1.25 / (1.0 - FragColor.a));\n"
		"\t\t\tint phase  = (y % stride) * (stride >> 3);\n"
		"\t\t\tint step = x + (y * (stride >> 1)) + phase;\n"
		"\t\t\tif (step % stride == 0) discard;\n"
		"\t\t}\n"
		"\t}\n"
		"\n"
		"\tFragColor.a = 1.0;\n"
		"}";
}

/* ========== SHADER LOADING					==========	*/
VGFXAPI vBOOL vGFXLoadShader(vPCHAR buffer, vUI32 bufferSize,
	const vPCHAR shaderPath)
{
	vLogInfoFormatted(__func__, "Loading shader with path: '%s'.", shaderPath);
	
	/* open file */
	HANDLE fileHandle = vFileOpen(shaderPath);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		vLogError(__func__, "Could not open shader file.");
		return FALSE;
	}

	/* read to buffer */
	vBOOL readStatus = vFileRead(fileHandle, ZERO, bufferSize, buffer);
	if (readStatus == FALSE)
	{
		vLogError(__func__, "Could not read from shader file.");
		return FALSE;
	}

	/* close handle */
	vFileClose(fileHandle);

	vLogInfo(__func__, "Loaded shader.");
	return TRUE;
}


/* ========== SHADER COMPILING					==========	*/
VGFXAPI GLuint vGFXCompileShader(GLenum shaderType, PCHAR source)
{
	vLogInfo(__func__, "Compiling shader.");

	GLuint shaderID;

	/* compile shader */
	shaderID = glCreateShader(shaderType);
	glShaderSource(shaderID, 1, &source, ZERO);
	glCompileShader(shaderID);

	/* check if shader compiled properly */
	GLint compileStatus;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == FALSE)
	{
		vLogError(__func__, "Shader failed to compile.");
		GLsizei writeLength; /* unused */
		GLchar  errorBuff[BUFF_LARGE];
		glGetShaderInfoLog(shaderID, sizeof(errorBuff), &writeLength, errorBuff);
		vLogError(__func__, errorBuff);
		
		return ZERO;
	}

	return shaderID;
}


/* ========== SHADER PROGRAM CREATION			==========	*/
VGFXAPI GLuint vGFXCreateShaderProgram(GLuint vertex, GLuint frag)
{
	vLogInfo(__func__, "Creating shader program.");

	/* create new program and link vert & frag to it */
	GLuint shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertex);
	glAttachShader(shaderProgramID, frag);
	glLinkProgram(shaderProgramID);

	/* check linking status */
	GLuint linkStatus;
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &linkStatus);
	if (linkStatus == FALSE)
	{
		vLogError(__func__, "Shader program failed to link.");
		vCHAR errorBuffer[BUFF_LARGE];
		glGetProgramInfoLog(shaderProgramID, sizeof(errorBuffer), NULL, errorBuffer);
		vLogError(__func__, errorBuffer);
		return ZERO;
	}

	vLogInfo(__func__, "Shader program created.");
	return shaderProgramID;
}