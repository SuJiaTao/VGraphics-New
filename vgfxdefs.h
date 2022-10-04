
/* ========== <vgfxdefs.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* All macros, types and structs							*/

#ifndef _VGFX_DEFS_INCLUDE_
#define _VGFX_DEFS_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"
#include <gl/GL.h>

/* ========== TYPEDEFS							==========	*/
typedef void (*vGPFSHADERINIT)(struct vGShader* shader, vPTR shaderData, vPTR input);
typedef void (*vGPFSHADERRENDER)(struct vGShader* shader, vPTR shaderData, 
	struct vObject* object, struct vGMaterial* objectMaterial);

/* ========== SHADER OBJECT						==========	*/
typedef struct vGShader
{
	GLuint glHandle;	/* handle to openGL shader object */

	vPTR   shaderPersistentData;			/* persistent heap data	*/
	vUI64  shaderPersistentDataSizeBytes;	/* size of heap data	*/

	vGPFSHADERINIT   initFunc;		/* initialization function		*/
	vGPFSHADERRENDER renderFunc;	/* render function				*/
} vGShader, *vPGShader;


/* ========== MATERIAL AND RELATED				==========	*/
typedef struct vGQuad
{
	GLuint glHandle;	/* handle to GL object */
	const vPosition bottomLeft;
	const vPosition topLeft;
	const vPosition bottomRight;
	const vPosition topRight;
} vGQuad, *vPGQuad;

typedef struct vGTexture
{
	GLuint glHandle;	/* handle to GL object */
	
	vUI32  width;
	vUI32  height;

	/* skin system */
	vUI16  skinCount;
	vUI16  skinWidth;
	vUI16  skinHeight;
} vGTexture, *vPGTexture;

typedef struct vGColor
{
	float R;
	float G;
	float B;
	float A;
} vGColor, *vPGColor;

typedef struct vGMaterial
{
	vPGShader shader;	/* shader to use to render	*/

	vPGTexture texturePrimary;
	vPGTexture textureSecondary;

	vPGQuad meshQuad;
	vPGQuad uvQuad;

	vGColor tint;
} vGMaterial, *vPGMaterial;

/* ========== WINDOW STATE						==========	*/
typedef struct vGWindow
{
	HWND  window;
	HDC   deviceContext;
	HGLRC renderingContext;
	RECT  dimensions;
	float aspectRatio;
} vGWindow, *vPGWindow;

/* ========== MODULE INTERNALS					==========	*/
typedef struct _vGInternals
{
	vPWorker workerThread;		/* graphics thread		*/
	vGWindow window;			/* render window		*/


	vHNDL graphicsObjects;	/* dynamic list of all graphics components */

} _vGInternals, *_vPGInternals;

#endif
