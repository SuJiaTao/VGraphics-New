
/* ========== <vgfxdefs.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* All macros, types and structs							*/

#ifndef _VGFX_DEFS_INCLUDE_
#define _VGFX_DEFS_INCLUDE_

/* ========== INCLUDES							==========	*/
#include "vgfx.h"
#include <gl/GL.h>


/* ========== API DEFINITION					==========	*/
#ifdef VGRAPHICS_EXPORTS
#define VGFXAPI __declspec(dllexport)
#else
#define VGFXAPI __declspec(dllimport)
#endif


/* ========== TYPEDEFS							==========	*/
typedef void (*vPFGSHADERINIT)(struct vGShader* shader, vPTR shaderData, vPTR input);
typedef void (*vPFGSHADERRENDER)(struct vGShader* shader, vPTR shaderData, 
	struct vObject* object, struct vGMaterial* objectMaterial);


/* ========== SHADER OBJECT						==========	*/
typedef struct vGShader
{
	GLuint glHandle;	/* handle to openGL shader object */

	vPTR   shaderPersistentData;			/* persistent heap data	*/
	vUI64  shaderPersistentDataSizeBytes;	/* size of heap data	*/

	vPFGSHADERINIT   initFunc;		/* initialization function		*/
	vPFGSHADERRENDER renderFunc;	/* render function				*/
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
	
	vUI32 width;
	vUI32 height;

	vBOOL linearFilter : 1;
	vBOOL wrap		   : 1;

	/* skin system */
	vUI16 skinCount;
	vUI16 skinWidth;
	vUI16 skinHeight;
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

/* ========== INITIALIZATION PARAMETERS			==========	*/
typedef struct vGInitializeData
{
	vUI32 window_width;
	vUI32 window_height;
} vGInitializeData, *vPGInitializeData;

/* ========== MODULE INTERNALS					==========	*/
typedef struct vGDefaultShaders
{
	vPGShader shader_missing;
	vPGShader shader_rectangle;
	vPGShader shader_wobble;
} vGDefaultShaders, *vPGDefaultShaders;

typedef struct vGDefaultTextures
{
	vPGTexture texture_missing;
	vPGTexture texture_gradient;
	vPGTexture texture_devGray;
	vPGTexture texture_devOrange;
} vGDefaultTextures, *vPGDefaultTextures;

typedef struct vGDefaultQuads
{
	vPGQuad quad_normalized;
	vPGQuad quad_uv;
	vPGQuad quad_diamond;

	vPGQuad quad_normalized_1x2;
	vPGQuad quad_normalized_1x3;
	vPGQuad quad_normalized_2x3;
	vPGQuad quad_normalized_3x4;
} vGDefaultQuads, *vPGDefaultQuads;

typedef struct _vGInternals
{
	vPWorker workerThread;		/* graphics thread		*/
	vGWindow window;			/* render window		*/

	vHNDL textures;				/* fixed buffer of all textures */
	vGDefaultTextures defaultTextures;

	vHNDL quads;				/* fixed buffer of all quads	*/
	vGDefaultQuads defaultQuads;

	vHNDL shaders;				/* fixed buffer of all shaders	*/
	vGDefaultShaders defaultShaders;

	vUI16 materialComponentHandle;	/* handle to material component				*/
	vHNDL materialObjects;			/* dynamic list of all material components	*/

} _vGInternals, *_vPGInternals;
_vGInternals _vgfx;	/* INSTANCE */

#endif
