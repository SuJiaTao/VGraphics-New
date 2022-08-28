
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


/* ========== RENDER OBJECTS					==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(void)
{
	vPRenderObject rObj = vBufferAdd(_vgfx.renderObjectBuffer);
	rObj->render = TRUE;
	rObj->rectangle.width  = 1.0f;
	rObj->rectangle.height = 1.0f;
	rObj->transform.scale  = 1.0f;
	rObj->tint.R = 1.0f;
	rObj->tint.G = 1.0f;
	rObj->tint.B = 1.0f;
	rObj->tint.A = 1.0f;
	return rObj;
}

VGFXAPI vPRenderObject vGFXCreateRenderObjectR(vRect rect)
{
	vPRenderObject rObj = vGFXCreateRenderObject();
	rObj->rectangle = rect;
	return rObj;
}

VGFXAPI vPRenderObject vGFXCreateRenderObjectT(vRect rect, vPT2 transform)
{
	vPRenderObject rObj = vGFXCreateRenderObjectR(rect);
	vMemCopy(&rObj->transform, transform, sizeof(vT2));
	return rObj;
}

VGFXAPI vPRenderObject vGFXCreateRenderObjectEx(vRect rect, vPT2 transform, vPTexture texture)
{
	if (vhEnsureContext(__func__)) return NULL;

	vPRenderObject rObj = vGFXCreateRenderObjectT(rect, transform);
	vMemCopy(&rObj->texture, texture, sizeof(vTexture));
	return rObj;
}

VGFXAPI void vGFXDestroyRenderObject(vPRenderObject object)
{
	if (vhEnsureContext(__func__)) return;

	/* free associated gl objects */
	vGFXDestroyTexture(&object->texture);

	vBufferRemove(_vgfx.renderObjectBuffer, object);
}


/* ========== TEXTURES							==========	*/
VGFXAPI void vGFXCreateTexture(vPTexture outTexture, vUI32 width, vUI32 height, 
	vPBYTE byteData)
{
	if (vhEnsureContext(__func__)) return;

	/* set texture object members */
	vZeroMemory(outTexture, sizeof(vTexture));
	outTexture->totalWidth  = width;
	outTexture->totalHeight = height;

	/* generate texture with alpha support */
	glGenTextures(1, &outTexture->glHandle);
	glBindTexture(GL_TEXTURE_2D, outTexture->glHandle);
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGBA, width, height, ZERO, GL_RGBA, 
		GL_UNSIGNED_BYTE, byteData);

	/* forced wrap filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* forced linear filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

VGFXAPI void vGFXCreateTextureSkinned(vPTexture outTexture, vUI32 tWidth, vUI32 tHeight,
	vUI16 skinCount, vPBYTE byteData)
{
	if (vhEnsureContext(__func__)) return;

	vGFXCreateTexture(outTexture, tWidth, tHeight, byteData);
	outTexture->skinCount  = skinCount;
}

VGFXAPI void vGFXCreateTexturePNG(vPTexture outTexture, vUI32 width, vUI32 height,
	vUI16 skinCount, PCHAR filePath)
{
	if (vhEnsureContext(__func__)) return;

	vLogInfoFormatted(__func__, "Creating texture from file path: %s.", filePath);

	/* get file handle */
	HANDLE fileHndl = vFileOpen(filePath);
	if (fileHndl == INVALID_HANDLE_VALUE) 
	{
		vLogError(__func__, "Failed open file.");
		return;
	}

	/* load file into primary memory */
	vUI64 fileSizeBytes = vFileSize(fileHndl);
	vPBYTE fileBlock = vAlloc(fileSizeBytes);
	vBOOL result = vFileRead(fileHndl, 0, fileSizeBytes, fileBlock);
	if (result == FALSE)
	{
		vLogError(__func__, "Failed to read file.");
		return;
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

			vGFXCreateTextureSkinned(outTexture, width, height, skinCount,
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
}

VGFXAPI void vGFXDestroyTexture(vPTexture inTexture)
{
	if (vhEnsureContext(__func__)) return;
	glDeleteTextures(1, &inTexture->glHandle);
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