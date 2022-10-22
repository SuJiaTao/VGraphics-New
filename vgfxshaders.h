
/* ========== <vgfxshaders.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Default shader callbacks			 (internal access)		*/

#ifndef _VGFX_INTERNAL_SHADERS_INCLUDE_
#define _VGFX_INTERNAL_SHADERS_INLUCDE_ 

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== RECT SHADER						==========	*/
typedef struct vGS_rectData
{
	GLuint vertexAttribute;	/* vertex descriptor of buffer	*/
	GLuint baseRect;	/* vert buffer of basic rectangle	*/
} vGS_rectData, *vPGS_rectData;

void vGShader_rectInit(vPGShader shader, vPGS_rectData shaderData, vPTR input);
void vGShader_rectRender(vPGShader shader, vPGS_rectData shaderData,
	vPObject object, vPGRenderable renderData);
void vGShader_rectExit(vPGShader shader, vPTR shaderData);

#endif 
