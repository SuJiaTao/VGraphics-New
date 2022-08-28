
/* ========== <vgfxobjects.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "vgfxobjects.h"


/* ========== HELPER							==========	*/
static __forceinline void vhEnsureContext(PCHAR funcName)
{
	if (GetThreadId(GetCurrentThread()) != GetThreadId(_vgfx.renderThread))
	{
		vLogError(funcName, "Tried to call GFX function on non-render thread.");
	}
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


/* ========== RENDERABLE THINGS					==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(void)
{
	vhEnsureContext(__func__);

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
	vPRenderObject rObj = vGFXCreateRenderObjectT(rect, transform);
	vMemCopy(&rObj->texture, texture, sizeof(vTexture));
	return rObj;
}

VGFXAPI void vGFXDestroyRenderObject(vPRenderObject object)
{
	vhEnsureContext(__func__);

	/* free associated gl objects */
	vGFXDestroyTexture(&object->texture);

	vBufferRemove(_vgfx.renderObjectBuffer, object);
}

VGFXAPI void vGFXCreateTexture(vPTexture outTexture, vUI32 width, vUI32 height, 
	vPBYTE byteData)
{
	vhEnsureContext(__func__);

	/* generate texture with alpha support */
	glGenTextures(1, &outTexture->glHandle);
	glBindTexture(GL_TEXTURE_2D, outTexture->glHandle);
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGBA, width, height, ZERO, GL_RGBA, 
		GL_UNSIGNED_BYTE, byteData);

	/* forced linear filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

VGFXAPI void vGFXDestroyTexture(vPTexture inTexture)
{
	vhEnsureContext(__func__);
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