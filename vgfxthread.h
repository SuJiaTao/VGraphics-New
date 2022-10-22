
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
typedef struct vGRT_CShaderInput
{
	vPGShader shader;
	vPCHAR	  vertexSrc;
	vPCHAR	  fragSrc;
	vPTR	  userInput;
} vGRT_CShaderInput, *vPGRT_CShaderInput;
void vGRT_createShaderTask(vPWorker worker, vPTR workerData, vPGRT_CShaderInput input);
void vGRT_destroyShaderTask(vPWorker worker, vPTR workerData, vPTR input);
typedef struct vGRT_CSkinInput
{
	vPGSkin skin;
	vPBYTE  byteData;
} vGRT_CSkinInput, *vPGRT_CSkinInput;
void vGRT_createSkinTask(vPWorker worker, vPTR workerData, vPGRT_CSkinInput input);
void vGRT_destroySkinTask(vPWorker worker, vPTR workerData, vPGSkin input);




#endif
