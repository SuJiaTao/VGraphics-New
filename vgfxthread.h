
/* ========== <vgfxthread.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Render thread behavior (not meant for user acess)		*/

#ifndef _VGFX_INTERNAL_THREAD_INCLUDE_
#define _VGFX_INTERNAL_THREAD_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== RENDER THREAD FUNCTIONS			==========	*/
void vGRenderThread_initFunc(vPWorker worker, vPTR workerData, vPTR input);
void vGRenderThread_exitFunc(vPWorker worker, vPTR workerData);
void vGRenderThread_cycleFunc(vPWorker worker, vPTR workerData);



#endif
