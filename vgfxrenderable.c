
/* ========== <vgfxrenderable.c>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Rendererable component behavior (internal access)		*/

/* ========== INCLUDES							==========	*/
#include "vgfxrenderable.h"


/* ========== RENDER COMPONENT BEHAVIOR			==========	*/
void vGRenderable_initFunc(vPObject object, vPComponent component, vPTR input)
{
	/* get input copy (MUST BE FROM HEAP) */
	vPGRenderable inputCopy = input;

	/* refer to vgfxcore.c for renderableList adding behavior */
	vDBufferAdd(_vgfx.renderableList, component->objectAttribute);

	/* setup metadata */
	vPGRenderable renderableData = component->objectAttribute;
	renderableData->objectPtr = object;

	/* setup render-related members */
	renderableData->rect   = inputCopy->rect;
	renderableData->shader = inputCopy->shader;
	renderableData->skin   = inputCopy->skin;

	vFree(inputCopy);
}

void vGRenderable_destroyFunc(vPObject object, vPComponent component)
{
	vPGRenderable renderableData = component->objectAttribute;

	/* refer to vgfxcore.c for renderableList remove behavior */
	vDBufferRemove(_vgfx.renderableList,
		renderableData->internalStoredPtr);
}