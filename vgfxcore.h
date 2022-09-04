
/* ========== <vgfxcore.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Contains all core functions such as initialization,		*/
/* termination and syncing functions						*/

#ifndef _VGFX_CORE_INCLUDE_
#define _VGFX_CORE_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== INITIALIZATION AND TERMINATION	==========	*/
VGFXAPI vBOOL vGFXInitialize(void);
VGFXAPI vBOOL vGFXTerminate(void);

VGFXAPI vBOOL vGFXIsInitialized(void);


/* ========== THREAD SYNCHRONIZATION			==========	*/
VGFXAPI void vGFXLock(void);
VGFXAPI void vGFXUnlock(void);


/* ========== WINDOW FUNCTIONS					==========	*/
VGFXAPI void vGFXWindowSetTitle(vPCHAR title);
VGFXAPI void vGFXWindowSetSize(LONG width, LONG height);
VGFXAPI void vGFXWindowGetSize(LPLONG pWidth, LPLONG pHeight);


/* ========== CAMERA FUNCTIONS					==========	*/
VGFXAPI void vGFXCameraResetAll(void);
VGFXAPI void vGFXCameraSetPos(v2V position);
VGFXAPI void vGFXCameraSetRotation(float rotation);
VGFXAPI void vGFXCameraSetScale(float scale);
VGFXAPI void vGFXCameraSetTransform(vPT2 pTransform);
VGFXAPI void vGFXCameraGetTransform(vPT2 oTransform);


/* ========== INFORMATION POLLING				==========	*/
VGFXAPI vBOOL vGFXIsWindowClosed(void);
VGFXAPI vUI64 vGFXGetFrameCount(void);
VGFXAPI _vPGFXInternals vGFXGetInternals(void);

#endif