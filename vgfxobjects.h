
/* ========== <vgfxobjects.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* User-side GFX object manipulation functions				*/

#ifndef _VGFX_RENDER_OBJECT_INCLUDE_
#define _VGFX_RENDER_OBJECT_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== RENDERABLE THINGS					==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(void);
VGFXAPI void vGFXDestroyRenderObject(vPRenderObject object);
VGFXAPI void vGFXCreateTexture(vPTexture outTexture, vUI32 width, vUI32 height, 
	vPBYTE byteData);


/* ========== RENDER JOBS						==========	*/
VGFXAPI void vGFXCreateJob(vGFXPFRenderJob job, vPTR persistentData);

#endif
