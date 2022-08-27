
/* ========== <vgfxobjects.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "vgfxobjects.h"


/* ========== CREATION AND DESTRUCTION			==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(void)
{
	vPRenderObject rObj = vBufferAdd(_vgfx.renderObjectBuffer);
	rObj->render = TRUE;
	rObj->rectangle.width  = 1.0f;
	rObj->rectangle.height = 1.0f;
	rObj->transform.scale  = 1.0f;
	rObj->tint.R = 1.0f;
	rObj->tint.G = 1.0f;
	rObj->tint.B = 1.0f;
	rObj->tint.A = 1.0f;
	return rObj;
}

VGFXAPI void vGFXDestroyRenderObject(vPRenderObject object)
{
	vBufferRemove(_vgfx.renderObjectBuffer, object);
}
