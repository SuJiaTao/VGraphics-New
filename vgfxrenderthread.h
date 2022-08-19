
/* ========== <vgfxrenderthread.h>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* This header contains the render thread's entry point.	*/
/* this should not be executed by the user by any means.	*/

#ifndef _VGFX_RTHREAD_INCLUDE_
#define _VGFX_RTHREAD_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== RENDER THREAD ENTRY POINT			==========	*/
VGFXAPI void vGFXRenderThreadProcess(void* input);


#endif
