
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
	struct vObject* object, struct vGRenderable* renderData);


/* ========== MATERIAL AND RELATED				==========	*/
typedef struct vGSkin
{
	GLuint glHandle;

	vUI16  width;
	vUI16  height;
	vUI8 skinCount;

	vUI8 skinWidth;
	vUI8  skinHeight;
} vGSkin, *vPGSkin;

typedef struct vGRect
{
	float bottomLeft;
	float bottomRight;
	float topLeft;
	float topRight;
} vGRect, *vPGRect;

typedef struct vGColor
{
	float R;
	float G;
	float B;
	float A;
} vGColor, *vPGColor;

typedef struct vGShader
{
	GLuint glProgramHandle;

	vUI32  shaderDataSizeBytes;
	vPTR   shaderDataPtr;

	vPFGSHADERRENDER renderFunc;
} vGShader, *vPGShader;

typedef struct vGRenderable
{
	vPGShader shader;

	vPGSkin skin;
	vUI16   renderSkin;

	vGColor tint;
	vGRect  rect;
} vGRenderable, *vPGRenderable;


/* ========== WINDOW STATE						==========	*/
typedef struct vGWindow
{
	HWND  window;
	HDC   deviceContext;
	HGLRC renderingContext;
	RECT  dimensions;
} vGWindow, *vPGWindow;


/* ========== INITIALIZATION PARAMETERS			==========	*/
typedef struct vGInitializeData
{
	vUI32 window_width;
	vUI32 window_height;
} vGInitializeData, *vPGInitializeData;


/* ========== MODULE INTERNALS					==========	*/
typedef struct _vGInternals
{
	vPWorker workerThread;		/* graphics thread		*/
	vGWindow window;			/* render window		*/

	vHNDL shaderList;			/* static list of all shaders */
	vHNDL skinList;				/* static list of all skins	  */

	vHNDL renderableList;		/* dynamic list of all renderable objects */
} _vGInternals, *_vPGInternals;

_vGInternals _vgfx;	/* INSTANCE */


#endif
