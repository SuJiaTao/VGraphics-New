
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
#define WORKER_WAITTIME_MAX			UINT64_MAX

#define WINDOW_WIDTH_MIN	0x180
#define WINDOW_HEIGHT_MIN	0x180

#define CAMERA_TRANSFORM_STACK_SIZE	0x80

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
typedef void (*vPFGSHADEREXIT)(struct vGShader* shader, vPTR shaderData);


/* ========== ENUMS								==========	*/
typedef enum vGDefaultShader
{
	vGDefaultShader_Error,
	vGDefaultShader_Rect,
} vGDefaultShader;


/* ========== MATERIAL AND RELATED				==========	*/
typedef struct vGSkin
{
	GLuint glHandle; /* openGL texture object */

	vBOOL wrapped;
	vUI16 width;
	vUI16 height;
	vUI8  skinCount; /* starts from 0 */
} vGSkin, *vPGSkin;

typedef struct vGRect
{
	float left;
	float right;
	float bottom;
	float top;
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
	GLuint glVertHandle;
	GLuint glFragHandle;
	GLuint glProgramHandle;

	vUI32  shaderDataSizeBytes;
	vPTR   shaderDataPtr;

	vPFGSHADERINIT   initFunc;
	vPFGSHADERRENDER renderFunc;
	vPFGSHADEREXIT   exitFunc;
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

	vUI8 layer;
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
typedef struct vGDefaultShaderData
{
	GLuint vertexAttribute;	/* vertex descriptor of buffer		*/
	GLuint baseRect;		/* vert buffer of basic rectangle	*/
	GLuint missingTexture;	/* fallback texture					*/
} vGDefaultShaderData, *vPGDefaultShaderData;

typedef struct vGDefaultShaders
{
	vPGShader errShader;
	vPGShader rectShader;
} vGDefaultShaders, *vPGDefaultShaders;

typedef struct vGCameraTransformStack 
{
	vTransform stack[CAMERA_TRANSFORM_STACK_SIZE];
	vUI32	   ptr;
} vGCameraTransformStack, *vPGCameraTransformStack;

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

	/* default shader metadata */
	vGDefaultShaderData defaultShaderData;
	vGDefaultShaders defaultShaders;

	/* camera transform */
	vGCameraTransformStack cameraTransform;
} _vGInternals, *_vPGInternals;

_vGInternals _vgfx;	/* INSTANCE */


#endif
