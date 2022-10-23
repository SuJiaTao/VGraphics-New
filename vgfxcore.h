
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
VGFXAPI vPGRenderable vGCreateRenderable(vPObject object, vPGShader shader, 
	vPGSkin skin, vGRect rect);
VGFXAPI void vGDestroyRenderable(vPObject object);


/* ========== OBJECT CREATION AND DESTRUCTION	==========	*/
VGFXAPI vPGShader vGCreateShader(vPFGSHADERINIT initFunc, vPFGSHADERRENDER renderFunc,
	vPFGSHADEREXIT exitFunc, vUI32 shaderDataBytes, vPCHAR vertexSource, vPCHAR fragmentSource, 
	vPTR input);
VGFXAPI void vGDestroyShader(vPGShader shader);
VGFXAPI vPGSkin vGCreateSkinFromBytes(vUI16 width, vUI16 height, vUI8 skinCount,
	vPBYTE bytes);
VGFXAPI vPGSkin vGCreateSkinFromPNG(vUI16 width, vUI16 height, vUI8 skinCount,
	vPCHAR fileName);
VGFXAPI void vGDestroySkin(vPGSkin skin);
VGFXAPI vGRect vGCreateRect(float left, float right, float bottom, float top);
VGFXAPI vGRect vGCreateRectCentered(float widht, float height);


/* ========== INTERNALS							==========	*/
VGFXAPI _vPGInternals vGGetInternals(void);

#endif
