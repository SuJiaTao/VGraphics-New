
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


/* ========== MAGIC NUMBERS						==========	*/
#define SKINS_MAX					0x400
#define SHADERS_MAX					0x040
#define RENDERABLE_LIST_NODE_SIZE	0x200

/* ========== COLOR VALUES						==========	*/
#define VGFX_COLOR_0b  54,  90,  92
#define VGFX_COLOR_1b  77,  19, 109
#define VGFX_COLOR_2b 154, 184, 169
#define VGFX_COLOR_3b 233, 226, 171
#define VGFX_COLOR_4b 239, 187, 102

#define VGFX_COLOR_0f 0.211f, 0.352f, 0.360f
#define VGFX_COLOR_1f 0.301f, 0.074f, 0.427f
#define VGFX_COLOR_2f 0.603f, 0.721f, 0.662f
#define VGFX_COLOR_3f 0.913f, 0.886f, 0.670f
#define VGFX_COLOR_4f 0.937f, 0.733f, 0.400f


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
	vPTR objectPtr;
	vPTR internalStoredPtr;

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
	vPCHAR windowName;
	vUI32  windowWidth;
	vUI32  windowHeight;
	vUI32  targetFrameRate;
} vGInitializeData, *vPGInitializeData;


/* ========== MODULE INTERNALS					==========	*/
typedef struct _vGInternals
{
	CRITICAL_SECTION lock;

	vPWorker workerThread;		/* graphics thread		*/
	vUI32    targetFrameRate;
	vGWindow window;			/* render window		*/

	vUI16 renderableHandle;

	vHNDL shaderList;			/* static list of all shaders */
	vHNDL skinList;				/* static list of all skins	  */

	vHNDL renderableList;		/* dynamic list of all renderable objects */
} _vGInternals, *_vPGInternals;

_vGInternals _vgfx;	/* INSTANCE */


#endif
