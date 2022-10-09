
/* ========== <vgfxcore.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Core graphics functions									*/

#ifndef _VGFX_CORE_INCLUDE_
#define _VGFX_CORE_INLUCDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== INITIALIZATION					==========	*/
VGFXAPI vBOOL vGInitialize(vPGInitializeData initializationData);


/* ========== SYNCHRONIZATION					==========	*/
VGFXAPI void vGLock(void);
VGFXAPI void vGUnlock(void);

#endif
