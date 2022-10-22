
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


/* ========== RENDERABLE ATTACHMENT				==========	*/
VGFXAPI void vGCreateRenderable(vPObject object, vPGShader shader, vPGSkin skin, vGRect rect);
VGFXAPI void vGDestroyRenderable(vPObject object);


/* ========== OBJECT CREATION AND DESTRUCTION	==========	*/
VGFXAPI vPGShader vGCreateShader(vPFGSHADERINIT initFunc, vPFGSHADERRENDER renderFunc,
	vUI32 shaderDataBytes, vPCHAR vertexSource, vPCHAR fragmentSource, vPTR input);
VGFXAPI void vGDestroyShader(vPGShader shader);
VGFXAPI vPGSkin vGCreateSkinFromBytes(void);

#endif
