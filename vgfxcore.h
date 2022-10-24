
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
VGFXAPI vPGSkin vGCreateSkinFromBytes(vUI16 width, vUI16 height, vUI8 skinCount, vBOOL wrap,
	vPBYTE bytes);
VGFXAPI vPGSkin vGCreateSkinFromPNG(vUI16 width, vUI16 height, vUI8 skinCount, vBOOL wrap,
	vPCHAR fileName);
VGFXAPI void vGDestroySkin(vPGSkin skin);
VGFXAPI vGRect vGCreateRect(float left, float right, float bottom, float top);
VGFXAPI vGRect vGCreateRectCentered(float widht, float height);
VGFXAPI vGColor vGCreateColorF(float r, float g, float b, float a);
VGFXAPI vGColor vGCreateColorB(vBYTE r, vBYTE g, vBYTE b, vBYTE a);


/* ========== CAMERA MANIPULATION				==========	*/
VGFXAPI vTransform  vGCameraGetTransform(void);
VGFXAPI vPTransform vGCameraGetTransformPTR(void);
VGFXAPI void  vGCameraSetTransform(vTransform transform);
VGFXAPI vBOOL vGCameraPush(void);
VGFXAPI vBOOL vGCameraPop(vPTransform prevCamera);
VGFXAPI vUI32 vGCameraGetPointer(void);


/* ========== DEFAULT VALUES					==========	*/
VGFXAPI vPGShader vGGetDefaultShader(vGDefaultShader shaderType);
VGFXAPI vPGDefaultShaderData vGGetDefaultShaderData(void);


/* ========== INTERNALS							==========	*/
VGFXAPI _vPGInternals vGGetInternals(void);

#endif
