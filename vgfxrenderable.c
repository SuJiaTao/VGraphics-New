
/* ========== <vgfxrenderable.h					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Rendererable component behavior (internal access)		*/

/* ========== INCLUDES							==========	*/
#include "vgfxrenderable.h"


/* ========== RENDER COMPONENT BEHAVIOR			==========	*/
void vGRenderable_initFunc(vPObject object, vPComponent component)
{
	vDBufferAdd(_vgfx.renderableList, component->objectAttribute);
	vPGRenderable renderableData = component->objectAttribute;
	renderableData->objectPtr = object;
}

void vGRenderable_destroyFunc(vPObject object, vPComponent component)
{
	vPGRenderable renderableData = component->objectAttribute;
	vDBufferRemove(_vgfx.renderableList,
		renderableData->internalStoredPtr);
}