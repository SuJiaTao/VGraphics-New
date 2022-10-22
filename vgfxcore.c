
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
		vGRT_initFunc, vGRT_exitFunc, 
		vGRT_cycleFunc, NULL, initializationData);

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


/* ========== RENDERABLE ATTACHMENT				==========	*/
VGFXAPI void vGCreateRenderable(vPObject object, vPGShader shader, vPGSkin skin
	, vGRect rect)
{
	vPGRenderable input = vAllocZeroed(sizeof(vGRenderable));
	input->shader = shader;
	input->skin = skin;
	input->rect = rect;

	/* refer to vgfxrenderable.c for input behavior */
	vObjectAddComponent(object, _vgfx.renderableHandle, input);
}

VGFXAPI void vGDestroyRenderable(vPObject object)
{
	vObjectRemoveComponent(object, _vgfx.renderableHandle);
}


/* ========== OBJECT CREATION AND DESTRUCTION	==========	*/
VGFXAPI vPGShader vGCreateShader(vPFGSHADERINIT initFunc, vPFGSHADERRENDER renderFunc,
	vUI32 shaderDataBytes, vPCHAR vertexSource, vPCHAR fragmentSource, vPTR input)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	/* get shader object from buffer */
	vPGShader shaderObject = vBufferAdd(_vgfx.shaderList, NULL);
	shaderObject->initFunc = initFunc;
	shaderObject->renderFunc = renderFunc;
	shaderObject->shaderDataSizeBytes = shaderDataBytes;
	shaderObject->shaderDataPtr = vAllocZeroed(shaderDataBytes);

	/* shader input is ptr to shader object + src */
	vPGRT_CSTInput taskInput = vAllocZeroed(sizeof(vPGRT_CSTInput));
	taskInput->shader    = shaderObject;
	taskInput->fragSrc   = fragmentSource;
	taskInput->vertexSrc = vertexSource;
	taskInput->userInput = input;
	
	/* refer to vgfxrenderthread.c for input implementation */
	vTIME syncTick = 
		vWorkerDispatchTask(_vgfx.workerThread, vGRT_createShaderTask, taskInput);
	vWorkerWaitCycleCompletion(_vgfx.workerThread, syncTick, WORKER_WAITTIME_MAX);

	return shaderObject;
}

VGFXAPI void vGDestroyShader(vPGShader shader)
{

}

VGFXAPI vPGSkin vGCreateSkinFromBytes(void);