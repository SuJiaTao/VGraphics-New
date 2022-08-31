
/* ========== <vgfxobjects.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* User-side GFX object manipulation functions				*/

#ifndef _VGFX_RENDER_OBJECT_INCLUDE_
#define _VGFX_RENDER_OBJECT_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== LOGICAL OBJECTS					==========	*/
VGFXAPI vRect vGFXCreateRect(float width, float height);
VGFXAPI vColor4 vGFXCreateColor3(float R, float G, float B);
VGFXAPI vColor4 vGFXCreateColor4(float R, float G, float B, float A);
VGFXAPI v2V vGFXCreateVector2(float x, float y);
VGFXAPI vT2 vGFXCreateTransformF(float x, float y, float r, float s);
VGFXAPI vT2 vGFXCreateTransformV(v2V position, float r, float s);


/* ========== RENDER BUFFERS					==========	*/
VGFXAPI vPRenderBuffer vGFXCreateRenderBuffer(GLuint shader, vGFXPFRenderMethod renderMethod,
	vUI32 renderAttributeSize, vGFXPFRenderAttributeSetup renderAttributeSetup,
	vUI32 objectAttributeSize, vUI16 capacity);
VGFXAPI void vGFXDestroyRenderBuffer(vPRenderBuffer renderBuffer);
VGFXAPI void vGFXRenderBufferGetInfo(vPRenderBuffer renderBuffer, vPBufferInfo infoOut);

/* ========== RENDER OBJECTS					==========	*/
VGFXAPI vPRenderObject vGFXCreateRenderObject(vPRenderBuffer buff, vRect rect,
	vPTexture texture);
VGFXAPI vPRenderObject vGFXCreateRenderObjectT(vPRenderBuffer buff, vRect rect,
	vPTexture texture, vT2 transform);
VGFXAPI void vGFXDestroyRenderObject(vPRenderBuffer buff, vPRenderObject object);


/* ========== TEXTURES							==========	*/
VGFXAPI vPTexture vGFXCreateTexture(vUI32 width, vUI32 height,
	vPBYTE byteData);
VGFXAPI vPTexture vGFXCreateTextureSkinned(vUI32 tWidth, vUI32 tHeight,
	vUI16 skinCount, vPBYTE byteData);
VGFXAPI vPTexture vGFXCreateTexturePNG(vUI32 width, vUI32 height,
	vUI16 skinCount, PCHAR filePath);
VGFXAPI void vGFXDestroyTexture(vPTexture inTexture);


/* ========== RENDER JOBS						==========	*/
VGFXAPI void vGFXCreateRenderJob(vGFXPFRenderJob job, vPTR persistentData);

#endif
