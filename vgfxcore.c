
/* ========== <vgfxcore.c>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "vgfxcore.h"
#include "vgfxthread.h"
#include "vgfxrenderable.h"


/* ========== BUFFER CALLBACKS					==========	*/
static void vGRenderableList_initFunc(vHNDL buffer, vPGRenderable* element, 
	vPGRenderable input)
{
	/* set element to hold pointer to component's renderable object attribute */
	*element = input;

	/* set renderable to point back to element (for deletion later) */
	input->internalStoredPtr = element;
}


/* ========== INITIALIZATION					==========	*/
VGFXAPI vBOOL vGInitialize(vPGInitializeData initializationData)
{
	vLogInfo(__func__, "Initializing vGFX.");

	/* initialize module lock */
	InitializeCriticalSection(&_vgfx.lock);

	/* initialize buffers */
	_vgfx.skinList = vCreateBuffer("vGFX Skin List", sizeof(vGSkin), SKINS_MAX,
		NULL, NULL, NULL);
	_vgfx.shaderList = vCreateBuffer("vGFX Shader List", sizeof(vGShader), SHADERS_MAX,
		NULL, NULL, NULL);
	_vgfx.renderableList = vCreateDBuffer("vGFX Renderable List", sizeof(vGRenderable),
		RENDERABLE_LIST_NODE_SIZE, vGRenderableList_initFunc, NULL);

	/* start render worker thread */
	vTIME timeInterval = 0x400 / (initializationData->targetFrameRate);
	_vgfx.workerThread = vCreateWorker("vGFX Render Thread", timeInterval,
		vGRenderThread_initFunc, vGRenderThread_exitFunc, 
		vGRenderThread_cycleFunc, NULL, initializationData);

	/* create renderable behavior */
	_vgfx.renderableHandle = vCreateComponent("vGFX Renderable", ZERO, sizeof(vPGRenderable),
		NULL, vGRenderable_initFunc, vGRenderable_destroyFunc, NULL, _vgfx.workerThread);
}


/* ========== SYNCHRONIZATION					==========	*/
VGFXAPI void vGLock(void)
{
	EnterCriticalSection(&_vgfx.lock);
}

VGFXAPI void vGUnlock(void)
{
	LeaveCriticalSection(&_vgfx.lock);
}
