
/* ========== <vgfxcore.c>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "vgfxcore.h"
#include "vgfxrenderthread.h"


/* ========== INITIALIZATION AND TERMINATION	==========	*/
VGFXAPI vBOOL vGFXInitialize(void)
{
	if (_vgfx.initialized == TRUE)
	{
		vLogError(__func__, "VGFX Initialization failed. VGFX was already initialized.");
		return FALSE;
	}

	vLogInfo(__func__, "Initializing VGFX.");

	/* zero library data */
	vZeroMemory(&_vgfx, sizeof(_vgfx));
	_vgfx.initialized = TRUE;

	/* initialize vcore objects */
	_vgfx.renderThreadLock   = vCreateLock();
	_vgfx.renderObjectBuffer = vCreateBuffer("VGFX Render Object Buffer",
		sizeof(vRenderObject), RENDER_OBJECTS_MAX);
	_vgfx.jobBuffer.jobBufferLock = vCreateLock();
	vGFXCameraResetAll();

	vLogInfo(__func__, "Starting VGFX render thread.");

	/* start render thread (refer to <vgfxrenderthread.c>) */
	_vgfx.renderThread = CreateThread(NO_FLAGS, ZERO, vGFXRenderThreadProcess, NULL,
		NO_FLAGS, NULL);

	vLogInfo(__func__, "VGFX Initialized.");
	vDumpEntryBuffer();	/* dump for safe measures */

	return TRUE;
}

VGFXAPI vBOOL vGFXTerminate(void)
{
	if (_vgfx.initialized == FALSE)
	{
		vLogError(__func__, "VGFX Termination failed. VGFX was already terminated.");
		return FALSE;
	}

	/* signal thread to exit and wait for thread */
	_vgfx.killThreadSignal = TRUE;
	DWORD waitResult;
	for (int tryCount = 0; tryCount < VGFX_TERMINATE_TRIES_MAX; tryCount++)
	{
		waitResult = WaitForSingleObject(_vgfx.renderThread, VGFX_TERMINATE_TRYTIME_MSEC);
		if (waitResult == WAIT_OBJECT_0) break;

		/* on still waiting, log */
		vLogInfoFormatted(__func__, "Waiting for RenderThread to exit [%d/%d].",
			tryCount, VGFX_TERMINATE_TRIES_MAX);
	}

	/* on still waiting, fatal error */
	if (waitResult != WAIT_OBJECT_0)
	{
		vLogError(__func__, "RenderThread could not be closed. Exiting Process.");
		vCoreFatalError(__func__, "VGFX Terminate failed. RenderThread could not be closed.");
	}

	/* free vcore resources and zero memory */
	vDestroyLock(_vgfx.renderThreadLock);
	vDestroyBuffer(_vgfx.renderObjectBuffer);
	vDestroyLock(_vgfx.jobBuffer.jobBufferLock);
	vZeroMemory(&_vgfx, sizeof(_vgfx));

	vLogInfo(__func__, "VGFX Terminated.");
	vDumpEntryBuffer();

	return TRUE;
}

VGFXAPI vBOOL vGFXIsInitialized(void)
{
	return _vgfx.initialized;
}


/* ========== WINDOW FUNCTIONS					==========	*/
VGFXAPI void vGFXWindowSetTitle(vPCHAR title)
{
	SetWindowTextA(_vgfx.renderWindow, title);
}

VGFXAPI void vGFXWindowSetSize(LONG width, LONG height)
{
	SetWindowPos(_vgfx.renderWindow, NO_WINDOW, 0, 0, width, height, SWP_NOMOVE);
}

VGFXAPI void vGFXWindowGetSize(LPLONG pWidth, LPLONG pHeight)
{
	RECT wRect;
	GetWindowRect(_vgfx.renderWindow, &wRect);
	*pWidth  = wRect.right  - wRect.left;
	*pHeight = wRect.bottom - wRect.top;
}


/* ========== THREAD SYNCHRONIZATION			==========	*/
VGFXAPI void vGFXLock(void)
{
	vLock(_vgfx.renderThreadLock);
}

VGFXAPI void vGFXUnlock(void)
{
	vUnlock(_vgfx.renderThreadLock);
}


/* ========== CAMERA FUNCTIONS					==========	*/
VGFXAPI void vGFXCameraResetAll(void)
{
	_vgfx.cameraTransform.position.x = 0;
	_vgfx.cameraTransform.position.y = 0;
	_vgfx.cameraTransform.rotation = 0;
	_vgfx.cameraTransform.scale = 1;
}

VGFXAPI void vGFXCameraSetPos(v2V position)
{
	_vgfx.cameraTransform.position = position;
}

VGFXAPI void vGFXCameraSetRotation(float rotation)
{
	_vgfx.cameraTransform.rotation = rotation;
}

VGFXAPI void vGFXCameraSetScale(float scale)
{
	_vgfx.cameraTransform.scale = scale;
}

VGFXAPI void vGFXCameraSetTransform(vPT2 pTransform)
{
	vMemCopy(&_vgfx.cameraTransform, pTransform, sizeof(vT2));
}

VGFXAPI void vGFXCameraGetTransform(vPT2 oTransform)
{
	*oTransform = _vgfx.cameraTransform;
}


/* ========== INFORMATION POLLING				==========	*/
VGFXAPI vBOOL vGFXIsWindowClosed(void)
{
	return _vgfx.windowClosed;
}

VGFXAPI _vPGFXInternals vGFXGetLibraryPointer(void)
{
	return &_vgfx;
}
