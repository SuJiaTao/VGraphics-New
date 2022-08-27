
/* ========== <vgfxshaders.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Shader loading and creation logic for RenderThread		*/

#ifndef _VGFX_SHADERS_INCLUDE_
#define _VGFX_SHADERS_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== SHADER LOADING					==========	*/
VGFXAPI vBOOL vGFXLoadShader(vPCHAR buffer, vUI32 bufferSize,
	const vPCHAR shaderPath);


/* ========== SHADER COMPILING					==========	*/
VGFXAPI GLuint vGFXCompileShader(GLenum shaderType, PCHAR source);


/* ========== SHADER PROGRAM CREATION			==========	*/
VGFXAPI GLuint vGFXCreateShaderProgram(GLuint vertex, GLuint frag);


/* ========== SHADER VERTEX ATTRIBUTE SETUP		==========	*/
VGFXAPI void vGFXSetupShaderVertexAttributes(void);


#endif