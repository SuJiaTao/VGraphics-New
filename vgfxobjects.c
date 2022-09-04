
/* ========== <vgfxobjects.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "vgfxobjects.h"
#include <intrin.h>
#include <stdio.h>

/* ========== HELPER							==========	*/
static __forceinline vBOOL vhEnsureContext(PCHAR funcName)
{
	if (GetThreadId(GetCurrentThread()) != GetThreadId(_vgfx.renderThread))
	{
		vLogError(funcName, "Tried to call GFX function on non-render thread.");
		return TRUE;
	}
	return FALSE;
}


/* ========== LOGICAL OBJECTS					==========	*/
VGFXAPI vRect vGFXCreateRect(float width, float height)
{
	vRect rect = { width, height };
	return rect;
}

VGFXAPI vColor4 vGFXCreateColor3(float R, float G, float B)
{
	vColor4 color = { R, G, B, 1.0f };
	return color;
}

VGFXAPI vColor4 vGFXCreateColor4(float R, float G, float B, float A)
{
	vColor4 color = { R, G, B, A };
	return color;
}

VGFXAPI v2V vGFXCreateVector2(float x, float y)
{
	v2V rVector = { x, y };
	return rVector;
}

VGFXAPI vT2 vGFXCreateTransformF(float x, float y, float r, float s)
{
	vT2 rTransform = { {x, y}, s, r };
	return rTransform;
}

VGFXAPI vT2 vGFXCreateTransformV(v2V position, float r, float s)
{
	vT2 rTransform = { position, s, r };
	return rTransform;
}

/* ========== RENDER BEHAVIORS					==========	*/
VGFXAPI vPRenderBehavior vGFXGetDefaultRenderBehavior(void)
{
	return _vgfx.defaultRenderBehavior;
}

VGFXAPI vPRenderBehavior vGFXCreateRenderBehavior(GLuint shader, vGFXPFRenderMethod renderMethod,
	vUI32 renderAttributeSize, vGFXPFRenderAttributeSetup renderAttributeSetup,
	vUI32 objectAttributeSize)
{
	vGFXLock();

	/* create object */
	vPRenderBehavior bhv = vBufferAdd(_vgfx.renderBehaviorBuffer);
	vLogInfoFormatted(__func__, "Creating render behavior %02X",
		vBufferGetElementIndex(_vgfx.renderBehaviorBuffer, bhv));

	/* setup members */
	bhv->shader = shader;
	bhv->renderMethod = renderMethod;
	bhv->renderAttributeSizeBytes = renderAttributeSize;
	bhv->renderAttributeSetup = renderAttributeSetup;
	bhv->objectAttributeSizeBytes = objectAttributeSize;
	bhv->renderAttributePtr = vAllocZeroed(max(renderAttributeSize, 
		RENDERATTRIBUTE_SIZE_MIN));

	/* call initialization method */
	if (bhv->renderAttributeSetup != NULL)
	{
		bhv->renderAttributeSetup(bhv, bhv->renderAttributePtr);
	}

	/* log warnings if possible */
	if (renderMethod == NULL) vLogWarning(__func__, "Created RenderBehavior with NULL render method.");

	vGFXUnlock();

	return bhv;
}

VGFXAPI void vGFXDestroyRenderBehavior(vPRenderBehavior behavior)
{
	vGFXLock();

	vLogInfoFormatted(__func__, "Destroying render behavior %02X.",
		vBufferGetElementIndex(_vgfx.renderBehaviorBuffer, behavior));

	/* free block and remove from buffer */
	vFree(behavior->renderAttributePtr);
	vBufferRemove(_vgfx.renderBehaviorBuffer, behavior);

	vGFXUnlock();
}


/* ========== RENDER BUFFERS					==========	*/
VGFXAPI vPRenderBuffer vGFXCreateRenderBuffer(vPRenderBehavior behavior, vUI16 capacity)
{
	vGFXLock();

	/* allocate buffer and setup render behavior */
	vPRenderBuffer renderBuffer = vBufferAdd(_vgfx.renderBuffers);
	renderBuffer->renderBehavior = behavior;

	/* format object buffer name */
	vCHAR bufferName[BUFF_MEDIUM];
	sprintf_s(bufferName, BUFF_MEDIUM, "Render Object Buffer %02X",
		vBufferGetElementIndex(_vgfx.renderBuffers, renderBuffer));

	/* initialize object buffer */
	renderBuffer->objectBuffer = vCreateBuffer(bufferName, sizeof(vRenderObject) +
		renderBuffer->renderBehavior->objectAttributeSizeBytes, capacity);

	vGFXUnlock();

	return renderBuffer;
}

VGFXAPI void vGFXDestroyRenderBuffer(vPRenderBuffer renderBuffer)
{
	vGFXLock();

	/* free associated buffer */
	vDestroyBuffer(renderBuffer->objectBuffer);

	/* remove from render buffers */
	vBufferRemove(_vgfx.renderBuffers, renderBuffer);

	vGFXUnlock();
}

VGFXAPI void vGFXRenderBufferGetInfo(vPRenderBuffer renderBuffer, vPBufferInfo infoOut)
{
	vBufferGetInfo(renderBuffer->objectBuffer, infoOut);
}


/* ========== RENDER OBJECTS					==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(vPRenderBuffer buff, vRect rect,
	vPTexture texture)
{
	vPRenderObject rObj = vBufferAdd(buff->objectBuffer);

	/* set all members */
	rObj->renderBuffer   = buff;
	rObj->renderBehavior = buff->renderBehavior;
	rObj->render	= TRUE;
	rObj->rectangle = rect;
	rObj->texture   = texture;
	rObj->transform = vGFXCreateTransformF(0.0f, 0.0f, 0.0f, 1.0f);
	rObj->tint		= vGFXCreateColor3(1.0f, 1.0f, 1.0f);
	return rObj;
}

VGFXAPI vPRenderObject vGFXCreateRenderObjectT(vPRenderBuffer buff, vRect rect,
	vPTexture texture, vT2 transform)
{
	vPRenderObject rObj = vGFXCreateRenderObject(buff, rect, texture);
	rObj->transform = transform;
	return rObj;
}

VGFXAPI void vGFXDestroyRenderObject(vPRenderBuffer buff, vPRenderObject object)
{
	if (vhEnsureContext(__func__)) return;

	/* free associated gl objects */
	vGFXDestroyTexture(&object->texture);

	vBufferRemove(buff->objectBuffer, object);
}


/* ========== TEXTURES							==========	*/
VGFXAPI vPTexture vGFXCreateTexture(vUI32 width, vUI32 height,
	vPBYTE byteData)
{
	if (vhEnsureContext(__func__)) return NULL;

	vPTexture texObj = vBufferAdd(_vgfx.textureBuffer);

	/* set texture object members */
	vZeroMemory(texObj, sizeof(vTexture));
	texObj->totalWidth  = width;
	texObj->totalHeight = height;

	/* generate texture with alpha support */
	glGenTextures(1, &texObj->glHandle);
	glBindTexture(GL_TEXTURE_2D, texObj->glHandle);
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGBA, width, height, ZERO, GL_RGBA, 
		GL_UNSIGNED_BYTE, byteData);

	/* forced wrap filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* forced linear filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return texObj;
}

VGFXAPI vPTexture vGFXCreateTextureSkinned(vUI32 tWidth, vUI32 tHeight,
	vUI16 skinCount, vPBYTE byteData)
{
	if (vhEnsureContext(__func__)) return NULL;

	vPTexture texObj = vGFXCreateTexture(tWidth, tHeight, byteData);
	texObj->skinCount  = skinCount;
	return texObj;
}

VGFXAPI vPTexture vGFXCreateTexturePNG(vUI32 width, vUI32 height,
	vUI16 skinCount, PCHAR filePath)
{
	if (vhEnsureContext(__func__)) return NULL;

	/* texture Object to return */
	vPTexture texObj = NULL;

	vLogInfoFormatted(__func__, "Creating texture from file path: %s.", filePath);

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
			vPBYTE imageData   = fileBlock + (readPointer + 8);
			vUI64  imageByteIndex = 0;
			for (int i = 0; i < height; i++)
			{
				/* invert Y */
				int heightActual = height - i - 1;

				/* copy data */
				vPBYTE parseBlockWritePtr = parsedBlock + (heightActual * width * 4);
				vPBYTE imageDataReadPtr   = imageData + imageByteIndex;
				vMemCopy(parseBlockWritePtr, imageDataReadPtr, width * 4);

				/* image byte index is incremented by the width + 1 to account for	*/
				/* the extra filter byte. what a pain!								*/
				imageByteIndex = imageByteIndex + (width * 4) + 1;
			}

			/* create texture using parsed block */
			texObj = vGFXCreateTextureSkinned(width, height, skinCount,
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

	vLogInfoFormatted(__func__, "Texture created from PNG!");
	return texObj;
}

VGFXAPI void vGFXDestroyTexture(vPTexture inTexture)
{
	if (vhEnsureContext(__func__)) return;
	glDeleteTextures(1, &inTexture->glHandle);
	vBufferRemove(_vgfx.textureBuffer, inTexture);
}


/* ========== RENDER JOBS						==========	*/
VGFXAPI void vGFXCreateRenderJob(vGFXPFRenderJob job, vPTR persistentData)
{
	/* wait for there to be free job spots */
	while (TRUE)
	{
		/* LOCK BUFFER */ vLock(_vgfx.jobBuffer.jobBufferLock);

		/* on free spot, break */
		if (_vgfx.jobBuffer.jobsLeftToExecute < RENDERJOBS_PER_CYCLE) break;

		/* else, unlock and sleep until it's time to check again */
		/* UNLOCK BUFFER */ vUnlock(_vgfx.jobBuffer.jobBufferLock);
		Sleep(RENDERJOB_WAIT_MSEC);
	}
	
	/* get next index in circular array */
	vUI64 nextJobIndex = _vgfx.jobBuffer.jobStartIndex + (vUI64)_vgfx.jobBuffer.jobsLeftToExecute;
	vPRenderJob pJob = _vgfx.jobBuffer.jobs + (nextJobIndex % RENDERJOBS_PER_CYCLE);
	
	/* set job and increment job counter */
	pJob->job  = job;
	pJob->data = persistentData;
	_vgfx.jobBuffer.jobsLeftToExecute++;
	
	/* UNLOCK BUFFER */ vUnlock(_vgfx.jobBuffer.jobBufferLock);
}