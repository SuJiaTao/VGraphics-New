
/* ========== <vgfxcore.c>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include <stdio.h>
#include "vgfxcore.h"
#include "vgfxthread.h"
#include "vgfxrenderable.h"
#include "vgfxshaders.h"


/* ========== BUFFER CALLBACKS					==========	*/
static void vGRenderableList_initFunc(vHNDL buffer, vPGRenderable* element,
	vPGRenderable input)
{
	/* each element holds a pointer to a renderable object */
	/* a pointer to the element is passed to this function */
	/* therefore element is a renderable 2xpointer		   */

	/* set element to hold pointer to component's renderable object attribute */
	*element = input;

	/* set renderable to point back to element (for deletion later) */
	input->internalStoredPtr = element;
}

static void vGExitCallbackList_initFunc(vHNDL buffer, vUI16 index,
	vPFBUFFERINITIALIZEELEMENT* element, vPFBUFFERINITIALIZEELEMENT input)
{
	/* we are passed a pointer to the element	*/
	/* each element is a fptr to callback		*/
	*element = input;
}

/* ========== INITIALIZATION					==========	*/
VGFXAPI vBOOL vGInitialize(vPGInitializeData initializationData)
{
	vLogInfo(__func__, "Initializing vGFX.");

	/* initialize module lock */
	InitializeCriticalSection(&_vgfx.lock);

	/* initialize buffers */
	_vgfx.skinList = vCreateBuffer("vGFX Skin List", sizeof(vGSkin), SKINS_MAX,
		NULL, NULL);

	_vgfx.shaderList = vCreateBuffer("vGFX Shader List", sizeof(vGShader), SHADERS_MAX,
		NULL, NULL);
	
	_vgfx.renderableList = vCreateDBuffer("vGFX Renderable List", sizeof(vPGRenderable),
		RENDERABLE_LIST_NODE_SIZE, vGRenderableList_initFunc, NULL);

	_vgfx.exitCallbackList = vCreateBuffer("vGFX Exit Callback List", sizeof(vPFGEXITCALLBACK),
		EXIT_CALLBACK_LIST_SIZE, vGExitCallbackList_initFunc, NULL);

	/* MAKE COPY OF INIT DATA ON HEAP FOR THREAD PERSISTENCE */
	vPGInitializeData heapDataCopy = vAllocZeroed(sizeof(vGInitializeData));
	vMemCopy(heapDataCopy, initializationData, sizeof(vGInitializeData));

	/* start render worker thread */
	vTIME timeInterval = 0x400 / (initializationData->targetFrameRate);
	_vgfx.workerThread = vCreateWorker("vGFX Render Thread", timeInterval,
		vGRT_initFunc, vGRT_exitFunc, 
		vGRT_cycleFunc, NULL, heapDataCopy);

	/* ensure the worker completes it's few cycles */
	vWorkerWaitCycleCompletion(_vgfx.workerThread, WORKER_WARMUP_CYCLES, 
		WORKER_WAITTIME_MAX);

	/* create renderable behavior */
	_vgfx.renderableHandle = vCreateComponent("vGFX Renderable", ZERO, sizeof(vGRenderable),
		NULL, vGRenderable_initFunc, vGRenderable_destroyFunc, NULL, NULL);

	/* setup default camera transform */
	vGCameraSetTransform(vCreateTransformF(0.0f, 0.0f, 0.0f, 1.0f));

	/* setup default shaders */
	_vgfx.defaultShaders.errShader =
		vGCreateShader(NULL, vGShader_errRender, NULL,
			NULL, vGShader_errRenderGetVert(), vGShader_errRenderGetFrag(), NULL);
	_vgfx.defaultShaders.rectShader =
		vGCreateShader(NULL, vGShader_rectRender, NULL,
			NULL, vGShader_rectRenderGetVert(), vGShader_rectRenderGetFrag(), NULL);

	/* initialize line system */
	vGLineSystemInit();

	/* DUMP ALL INFO */
	vDumpEntryBuffer();
}

VGFXAPI vGRect vGCreateRect(float left, float right, float bottom, float top)
{
	vGRect rect;
	rect.left = left;
	rect.right = right;
	rect.bottom = bottom;
	rect.top = top;
	return rect;
}

VGFXAPI vGRect vGCreateRectCentered(float width, float height)
{
	vGRect rect;
	rect.left   = -width / 2.0f;
	rect.right  =  width / 2.0f;
	rect.bottom = -height / 2.0f;
	rect.top    =  height / 2.0f;
	return rect;
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
VGFXAPI vPGRenderable vGCreateRenderable(vPObject object, vTransform transform,
	vPGShader shader, vPGSkin skin, vGRect rect, vPGRenderableBehavior behavior)
{
	vPGRenderable input = vAllocZeroed(sizeof(vGRenderable));

	input->objectPtr = object; /* setup metadata */

	/* setup render data */
	input->shader = shader;
	input->skin = skin;
	input->rect = rect;
	input->tint = vGCreateColorF(1.0f, 1.0f, 1.0f, 1.0f);
	input->transform = transform;

	/* setup object behavior */
	if (behavior != NULL)
		vMemCopy(&input->behavior, behavior, sizeof(vGRenderableBehavior));

	/* refer to vgfxrenderable.c for input behavior */
	vPComponent renderComponent = 
		vObjectAddComponent(object, _vgfx.renderableHandle, input);

	return renderComponent->objectAttribute;
}

VGFXAPI void vGDestroyRenderable(vPObject object)
{
	vObjectRemoveComponent(object, _vgfx.renderableHandle);
}

VGFXAPI vUI16 vGGetComponentHandle(void)
{
	return _vgfx.renderableHandle;
}


/* ========== OBJECT CREATION AND DESTRUCTION	==========	*/
VGFXAPI vPGShader vGCreateShader(vPFGSHADERINIT initFunc, vPFGSHADERRENDER renderFunc,
	vPFGSHADEREXIT exitFunc, vUI32 shaderDataBytes, vPCHAR vertexSource, vPCHAR fragmentSource,
	vPTR input)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	/* get shader object from buffer */
	vBufferLock(_vgfx.shaderList);
	vPGShader shaderObject   = vBufferAdd(_vgfx.shaderList, NULL);
	shaderObject->initFunc   = initFunc;
	shaderObject->renderFunc = renderFunc;
	shaderObject->exitFunc   = exitFunc;
	shaderObject->shaderDataSizeBytes = shaderDataBytes;
	shaderObject->shaderDataPtr = vAllocZeroed(max(4, shaderDataBytes));

	/* shader input is ptr to shader object + src */
	/* TASKINPUT IS FREED BY RENDER THREAD */
	vPGRT_CShaderInput taskInput = vAllocZeroed(sizeof(vGRT_CShaderInput));
	taskInput->shader    = shaderObject;
	taskInput->fragSrc   = fragmentSource;
	taskInput->vertexSrc = vertexSource;
	taskInput->userInput = input;
	vBufferUnlock(_vgfx.shaderList);
	
	/* refer to vgfxrenderthread.c/h for input implementation */
	vTIME syncTick = 
		vWorkerDispatchTask(_vgfx.workerThread, vGRT_createShaderTask, taskInput);
	vWorkerWaitCycleCompletion(_vgfx.workerThread, syncTick, WORKER_WAITTIME_MAX);

	return shaderObject;
}

VGFXAPI void vGDestroyShader(vPGShader shader)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	/* refer to vgfxrenderthread.c/h for input implementation */
	vTIME syncTick =
		vWorkerDispatchTask(_vgfx.workerThread, vGRT_destroyShaderTask, shader);
	vWorkerWaitCycleCompletion(_vgfx.workerThread, syncTick, WORKER_WAITTIME_MAX);

	/* free shader data */
	vFree(shader->shaderDataPtr);

	/* remove shader from buffer */
	vBufferRemove(_vgfx.shaderList, shader);
}

VGFXAPI vPGSkin vGCreateSkinFromBytes(vUI16 width, vUI16 height, vUI8 skinCount, vBOOL wrap,
	vPBYTE bytes)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	vPGSkin skin = vBufferAdd(_vgfx.skinList, NULL);
	skin->width = width;
	skin->height = height;
	skin->skinCount = skinCount;
	skin->wrapped = wrap;

	/* refer to vgfxrenderthread.c/h for input implementation	*/
	/* INPUT IS FREED BY RENDER THREAD							*/
	vPGRT_CSkinInput input = vAllocZeroed(sizeof(vGRT_CSkinInput));
	input->skin     = skin;
	input->byteData = bytes;

	/* refer to vgfxrenderthread.c/h for input implementation */
	vTIME syncTick =
		vWorkerDispatchTask(_vgfx.workerThread, vGRT_createSkinTask, input);
	vWorkerWaitCycleCompletion(_vgfx.workerThread, syncTick, WORKER_WAITTIME_MAX);

	return skin;
}

VGFXAPI vPGSkin vGCreateSkinFromPNG(vUI16 width, vUI16 height, vUI8 skinCount, vBOOL wrap,
	vPCHAR filePath)
{
	/* REQUIRES UNCOMPRESSED PNG, LATER CALLS vGCreateSkinFromBytes */

	/* skin object */
	vPGSkin skin = NULL;

	vLogInfoFormatted(__func__, "Creating skin from file path: %s.", filePath);

	/* get file handle */
	HANDLE fileHndl = vFileOpen(filePath);
	if (fileHndl == INVALID_HANDLE_VALUE)
	{
		vLogError(__func__, "Failed open file.");
		return NULL;
	}

	/* load file into primary memory */
	vUI64 fileSizeBytes = vFileSize(fileHndl);
	vPBYTE fileBlock = vAlloc(fileSizeBytes);
	vBOOL result = vFileRead(fileHndl, 0, fileSizeBytes, fileBlock);
	if (result == FALSE)
	{
		vLogError(__func__, "Failed to read file.");
		return NULL;
	}

	vUI32 readPointer = 8;	/* move past header */
	while (TRUE)
	{
		/* get chunk length */
		vUI32 blockLength;
		vMemCopy(&blockLength, fileBlock + readPointer, sizeof(blockLength));
		blockLength = _byteswap_ulong(blockLength); /* swap endian */

		readPointer += 4; /* move up 4 bytes */

		/* get chunk type */
		vCHAR blockName[5];	/* size 4 with null padding */
		blockName[4] = 0;
		vMemCopy(blockName, fileBlock + readPointer, 4);

		readPointer += 4; /* move up 4 bytes */

		/* if end chunk, break */
		if (strcmp(blockName, "IEND") == ZERO) break;

		/* if data chunk, parse data to remove all filter bytes */
		if (strcmp(blockName, "IDAT") == ZERO)
		{
			vPBYTE parsedBlock = vAlloc(width * height * 4);
			vPBYTE imageData = fileBlock + (readPointer + 8);
			vUI64  imageByteIndex = 0;
			for (int i = 0; i < height; i++)
			{
				/* invert Y */
				int heightActual = height - i - 1;

				/* copy data */
				vPBYTE parseBlockWritePtr = parsedBlock + (heightActual * width * 4);
				vPBYTE imageDataReadPtr = imageData + imageByteIndex;
				vMemCopy(parseBlockWritePtr, imageDataReadPtr, width * 4);

				/* image byte index is incremented by the width + 1 to account for	*/
				/* the extra filter byte. what a pain!								*/
				imageByteIndex = imageByteIndex + (width * 4) + 1;
			}

			/* create texture using parsed block */
			skin = vGCreateSkinFromBytes(width, height, skinCount, wrap,
				parsedBlock);

			/* free memory and break */
			vFree(parsedBlock);
			break;
		}

		/* move to next chunk */
		readPointer = readPointer + blockLength + 4;
	}

	/* free data */
	vFree(fileBlock);
	vFileClose(fileHndl);

	vLogInfoFormatted(__func__, "Skin created from PNG!");
	return skin;
}

VGFXAPI void vGDestroySkin(vPGSkin skin)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	/* refer to vgfxrenderthread.c/h for input implementation */
	vTIME syncTick =
		vWorkerDispatchTask(_vgfx.workerThread, vGRT_destroySkinTask, skin);
	vWorkerWaitCycleCompletion(_vgfx.workerThread, syncTick, WORKER_WAITTIME_MAX);

	vBufferRemove(_vgfx.skinList, skin);
}

VGFXAPI vGColor vGCreateColorF(float r, float g, float b, float a)
{
	vGColor rColor;
	rColor.R = r;
	rColor.G = g;
	rColor.B = b;
	rColor.A = a;
	return rColor;
}

VGFXAPI vGColor vGCreateColorB(vBYTE r, vBYTE g, vBYTE b, vBYTE a)
{
	vGColor color;
	color.R = (float)r / 255.0;
	color.G = (float)g / 255.0;
	color.B = (float)b / 255.0;
	color.A = (float)a / 255.0;
	return color;
}


/* ========== CAMERA MANIPULATION				==========	*/
VGFXAPI vTransform vGCameraGetTransform(void) 
{
	return _vgfx.cameraTransform;
}

VGFXAPI vPTransform vGCameraGetTransformPTR(void)
{
	return &_vgfx.cameraTransform;
}

VGFXAPI void  vGCameraSetTransform(vTransform transform)
{
	_vgfx.cameraTransform = transform;
}


/* ========== DEFAULT VALUES					==========	*/
VGFXAPI vPGShader vGGetDefaultShader(vGDefaultShader shaderType)
{
	switch (shaderType)
	{
	case vGDefaultShader_Error:
		return _vgfx.defaultShaders.errShader;

	case vGDefaultShader_Rect:
		return _vgfx.defaultShaders.rectShader;

	default:
		break;
	}

	vLogError(__func__, "Tried to retrieve invalid default shader type.");
	return NULL;
}

VGFXAPI vPGDefaultShaderData vGGetDefaultShaderData(void)
{
	return &_vgfx.defaultShaderData;
}


/* ========== EXIT RELATED						==========	*/
VGFXAPI void vGAttatchExitCallback(vPFGEXITCALLBACK exitFunc)
{
	vBufferAdd(_vgfx.exitCallbackList, exitFunc);
}

VGFXAPI void vGExit(void)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* ONLY THREADS OWNING A WINDOW CAN DESTROY THE WINDOW			*/
	vTIME syncTick =
		vWorkerDispatchTask(_vgfx.workerThread, vGRT_destroyWindowTask, NULL);
	vWorkerWaitCycleCompletion(_vgfx.workerThread, syncTick, WORKER_WAITTIME_MAX);
}

VGFXAPI vBOOL vGExited(void)
{
	return vWorkerIsAlive(_vgfx.workerThread) == FALSE;
}


/* ========== INTERNALS							==========	*/
VGFXAPI _vPGInternals vGGetInternals(void)
{
	return &_vgfx;
}