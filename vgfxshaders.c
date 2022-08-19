
/* ========== <vgfxshaders.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Shader loading and creation logic for RenderThread		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfx.h"


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
		vLogErrorFormatted(__func__, "Shader linking info: \n%s", errorBuffer);
		return ZERO;
	}

	vLogInfo(__func__, "Shader program created.");
	return shaderProgramID;
}