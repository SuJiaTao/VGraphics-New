
/* ========== <vgfxcore.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Core graphics functions									*/

#ifndef _VGFX_CORE_INCLUDE_
#define _VGFX_CORE_INLUCDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== INITIALIZATION AND WINDOW			==========	*/
VGFXAPI vBOOL vGInitialize(vPGInitializeData initializationData);
VGFXAPI void  vGSetWindowResizable(vBOOL state);
VGFXAPI void  vGSetWindowMaximizeable(vBOOL state);
VGFXAPI void  vGSetWindowSize(vUI32 width, vUI32 height);
VGFXAPI void  vGGetWindowSize(vPUI32 width, vPUI32 height);
VGFXAPI float vGGetWindowAspect(void);
VGFXAPI void  vGGetWindowClientSize(vPUI32 width, vPUI32 height);
VGFXAPI float vGGetWindowClientAspect(void);


/* ========== SYNCHRONIZATION					==========	*/
VGFXAPI void vGLock(void);
VGFXAPI void vGUnlock(void);
VGFXAPI void vGSync(void);
VGFXAPI void vGSyncWaitForFrames(vUI64 frameCount);


/* ========== RENDERABLE ATTACHMENT				==========	*/
VGFXAPI vPGRenderable vGCreateRenderable(vPObject object, vTransform transform,
	vPGShader shader, vPGSkin skin, vGRect rect, vPGRenderableBehavior behavior);
VGFXAPI void vGDestroyRenderable(vPObject object);
VGFXAPI vUI16 vGGetComponentHandle(void);


/* ========== OBJECT CREATION AND DESTRUCTION	==========	*/
VGFXAPI vPGShader vGCreateShader(vPFGSHADERINIT initFunc, vPFGSHADERRENDER renderFunc,
	vPFGSHADEREXIT exitFunc, vUI32 shaderDataBytes, vPCHAR vertexSource, vPCHAR fragmentSource, 
	vPTR input);
VGFXAPI void vGDestroyShader(vPGShader shader);
VGFXAPI vPGSkin vGCreateSkinFromBytes(vUI16 width, vUI16 height, vUI8 skinCount, vBOOL wrap,
	vPBYTE bytes);
VGFXAPI void vGDestroySkin(vPGSkin skin);
VGFXAPI vGRect vGCreateRect(float left, float right, float bottom, float top);
VGFXAPI vGRect vGCreateRectCentered(float widht, float height);
VGFXAPI vGColor vGCreateColorF(float r, float g, float b, float a);
VGFXAPI vGColor vGCreateColorB(vBYTE r, vBYTE g, vBYTE b, vBYTE a);


/* ========== CAMERA MANIPULATION				==========	*/
VGFXAPI vTransform  vGCameraGetTransform(void);
VGFXAPI vPTransform vGCameraGetTransformPTR(void);
VGFXAPI void  vGCameraSetTransform(vTransform transform);


/* ========== DEFAULT VALUES					==========	*/
VGFXAPI vPGShader vGGetDefaultShader(vGDefaultShader shaderType);
VGFXAPI vPGDefaultShaderData vGGetDefaultShaderData(void);


/* ========== EXIT RELATED						==========	*/
VGFXAPI void vGAttatchExitCallback(vPFGEXITCALLBACK exitFunc);
VGFXAPI void vGExit(void);
VGFXAPI vBOOL vGExited(void);
VGFXAPI vBOOL vGShouldExit(void);
VGFXAPI void  vGSetShouldExit(vBOOL value);


/* ========== INTERNALS							==========	*/
VGFXAPI _vPGInternals vGGetInternals(void);


/* ========== SPACIAL PROJECTIONS				==========	*/
VGFXAPI vPosition vGScreenSpaceMousePos(void);
VGFXAPI vPosition vGScreenSpaceToWorld(vPosition screenPos);
VGFXAPI vPosition vGWorldSpaceMousePosition(void);
VGFXAPI vPosition vGWorldSpaceToScreen(vPosition worldPos);

/* ========== VCI MANIPULATION					==========	*/
VGFXAPI vPBYTE  vGLoadVCI(vPCHAR path, vPUI32 width, vPUI32 height);
VGFXAPI void    vGMakeVCI(vPCHAR path, vUI32 width, vUI32 height, vPBYTE bytes);
VGFXAPI vPGSkin vGCreateSkinFromVCI(vPCHAR path, vBOOL wrap, vUI8 skins);


#endif
