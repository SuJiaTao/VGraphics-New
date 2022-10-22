
/* ========== <vgfxrenderable.h>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Rendererable component behavior (internal access)		*/

#ifndef _VGFX_INTERNAL_RENDERABLE_INCLUDE_
#define _VGFX_INTERNAL_RENDERABLE_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== RENDER COMPONENT BEHAVIOR			==========	*/
void vGRenderable_initFunc(vPObject object, vPComponent component, vPTR input);
void vGRenderable_destroyFunc(vPObject object, vPComponent component);


#endif
