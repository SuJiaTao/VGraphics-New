
/* ========== <vgfxshaders.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Default shader callbacks			 (internal access)		*/

#ifndef _VGFX_INTERNAL_SHADERS_INCLUDE_
#define _VGFX_INTERNAL_SHADERS_INLUCDE_ 

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== ERROR SHADER						==========	*/
vPCHAR vGShader_errRenderGetVert(void);
vPCHAR vGShader_errRenderGetFrag(void);
void vGShader_errRender(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData);


/* ========== RECT SHADER						==========	*/
void vGShader_rectRender(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData);

#endif 
