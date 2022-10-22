
/* ========== <vgfxthread.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Render thread behavior (not meant for user acess)		*/

#ifndef _VGFX_INTERNAL_THREAD_INCLUDE_
#define _VGFX_INTERNAL_THREAD_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== RENDER THREAD FUNCTIONS			==========	*/
void vGRT_initFunc(vPWorker worker, vPTR workerData, vPTR input);
void vGRT_exitFunc(vPWorker worker, vPTR workerData);
void vGRT_cycleFunc(vPWorker worker, vPTR workerData);


/* ========== TASKABLE FUNCTIONS				==========	*/
typedef struct vGRT_CSTInput
{
	vPGShader shader;
	vPCHAR	  vertexSrc;
	vPCHAR	  fragSrc;
	vPTR	  userInput;
} vGRT_CSTInput, *vPGRT_CSTInput;
void vGRT_createShaderTask(vPWorker worker, vPTR workerData, vPGRT_CSTInput input);
void vGRT_destroyShaderTask(vPWorker worker, vPTR workerData, vPTR input);




#endif
