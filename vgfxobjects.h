
/* ========== <vgfxobjects.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* User-side GFX object manipulation functions				*/

#ifndef _VGFX_RENDER_OBJECT_INCLUDE_
#define _VGFX_RENDER_OBJECT_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== CREATION AND DESTRUCTION			==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(void);
VGFXAPI void vGFXDestroyRenderObject(vPRenderObject object);

#endif
