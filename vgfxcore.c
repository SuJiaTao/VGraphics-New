
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
	vPFGSHADEREXIT exitFunc, vUI32 shaderDataBytes, vPCHAR vertexSource, vPCHAR fragmentSource,
	vPTR input)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	/* get shader object from buffer */
	vPGShader shaderObject = vBufferAdd(_vgfx.shaderList, NULL);
	shaderObject->initFunc = initFunc;
	shaderObject->renderFunc = renderFunc;
	shaderObject->exitFunc = exitFunc;
	shaderObject->shaderDataSizeBytes = shaderDataBytes;
	shaderObject->shaderDataPtr = vAllocZeroed(shaderDataBytes);

	/* shader input is ptr to shader object + src */
	/* TASKINPUT IS FREED BY RENDER THREAD */
	vPGRT_CShaderInput taskInput = vAllocZeroed(sizeof(vPGRT_CShaderInput));
	taskInput->shader    = shaderObject;
	taskInput->fragSrc   = fragmentSource;
	taskInput->vertexSrc = vertexSource;
	taskInput->userInput = input;
	
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

VGFXAPI vPGSkin vGCreateSkinFromBytes(vUI16 width, vUI16 height, vUI8 skinCount,
	vPBYTE bytes)
{
	/* THIS FUNCTION MUST BE DISPATCHED TO RENDER THREAD AS A TASK	*/
	/* AS IT REQUIRES AN OPENGL CONTEXT TO BE EXECUTED				*/

	vPGSkin skin = vBufferAdd(_vgfx.skinList, NULL);
	skin->width = width;
	skin->height = height;
	skin->skinCount = skinCount;

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

VGFXAPI vPGSkin vGCreateSkinFromPNG(vUI16 width, vUI16 height, vUI8 skinCount,
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
			skin = vGCreateSkinFromBytes(width, height, skinCount,
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