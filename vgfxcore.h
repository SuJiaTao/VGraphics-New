
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

/* ========== INFORMATION POLLING				==========	*/
VGFXAPI vBOOL vGFXIsWindowClosed(void);
VGFXAPI _vPGFXInternals vGFXGetLibraryPointer(void);

#endif