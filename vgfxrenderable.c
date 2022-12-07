
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

	/* get object attribute and copy data from input  */
	vPGRenderable renderableData = component->objectAttribute;
	vMemCopy(renderableData, input, sizeof(vGRenderable));

	/* refer to vgfxcore.c for renderableList adding behavior */
	vDBufferAdd(_vgfx.renderableList, component->objectAttribute);

	vFree(inputCopy);

	/* call initfunc */
	if (renderableData->behavior.initFunc)
		renderableData->behavior.initFunc(renderableData);
}

void vGRenderable_destroyFunc(vPObject object, vPComponent component)
{
	vPGRenderable renderableData = component->objectAttribute;

	/* call destroy func if exists */
	if (renderableData->behavior.exitFunc)
		renderableData->behavior.exitFunc(renderableData);

	/* refer to vgfxcore.c for renderableList remove behavior */
	vDBufferRemove(_vgfx.renderableList,
		renderableData->internalStoredPtr);
}