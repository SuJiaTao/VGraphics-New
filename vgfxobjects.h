
/* ========== <vgfxobjects.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* User-side GFX object manipulation functions				*/

#ifndef _VGFX_RENDER_OBJECT_INCLUDE_
#define _VGFX_RENDER_OBJECT_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== LOGICAL OBJECTS					==========	*/
VGFXAPI v2V vGFXCreateVector2(float x, float y);
VGFXAPI vT2 vGFXCreateTransformF(float x, float y, float r, float s);
VGFXAPI vT2 vGFXCreateTransformV(v2V position, float r, float s);


/* ========== RENDERABLE THINGS					==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(void);
VGFXAPI void vGFXDestroyRenderObject(vPRenderObject object);
VGFXAPI void vGFXCreateTexture(vPTexture outTexture, vUI32 width, vUI32 height, 
	vPBYTE byteData);
VGFXAPI void vGFXDestroyTexture(vPTexture inTexture);

/* ========== RENDER JOBS						==========	*/
VGFXAPI void vGFXCreateJob(vGFXPFRenderJob job, vPTR persistentData);

#endif
