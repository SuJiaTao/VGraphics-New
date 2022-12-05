
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


/* ========== MACROS							==========	*/
#define vGDebugMemValidate(x)	vFree(vAlloc(x))


/* ========== MAGIC NUMBERS						==========	*/
#define SKINS_MAX					0x400
#define SHADERS_MAX					0x040
#define RENDERABLE_LIST_NODE_SIZE	0x200
#define WORKER_WAITTIME_MAX			UINT64_MAX
#define WORKER_WARMUP_CYCLES		0x010

#define WINDOW_WIDTH_MIN	0x180
#define WINDOW_HEIGHT_MIN	0x180

#define CAMERA_TRANSFORM_STACK_SIZE	0x20

#define EXIT_CALLBACK_LIST_SIZE		0x100

#define VGFX_WINDOW_CLASS_NAME		"vGFX Window Class"

#define LINELIST_NODE_SIZE	0x200

#define ZNEAR_DEPTH			-1.0
#define ZFAR_DEPTH			 10.0
#define LINE_ZDEPTH			 0.5

/* ========== COLOR VALUES						==========	*/
#define VGFX_COLOR_0b  54,  90,  92
#define VGFX_COLOR_1b  77, 119, 109
#define VGFX_COLOR_2b 154, 184, 169
#define VGFX_COLOR_3b 233, 226, 171
#define VGFX_COLOR_4b 239, 187, 102

#define VGFX_COLOR_0f 0.211765f, 0.352941f, 0.360784f
#define VGFX_COLOR_1f 0.301961f, 0.466667f, 0.427451f
#define VGFX_COLOR_2f 0.603922f, 0.721569f, 0.662745f
#define VGFX_COLOR_3f 0.913725f, 0.886275f, 0.670588f
#define VGFX_COLOR_4f 0.937255f, 0.733333f, 0.400000f


/* ========== TYPEDEFS							==========	*/
typedef void (*vPFGSHADERINIT)(struct vGShader* shader, vPTR shaderData, vPTR input);
typedef void (*vPFGSHADERRENDER)(struct vGShader* shader, vPTR shaderData, 
	struct vObject* object, struct vGRenderable* renderData);
typedef void (*vPFGSHADEREXIT)(struct vGShader* shader, vPTR shaderData);
typedef void (*vPFGEXITCALLBACK)(void);


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

	vTransform transform;

	vPGShader shader;

	vPGSkin skin;
	vUI16   renderSkin;

	vGColor tint;
	vGRect  rect;

	vUI8 layer;
} vGRenderable, *vPGRenderable;

typedef struct vGLine
{
	vPosition p1;
	vPosition p2;
	vGColor   col;
	float     width;
} vGLine, *vPGLine;


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

typedef struct vGLineSystem
{
	vBOOL isInit;

	vHNDL lineList;				/* list of lines to draw (per cycle)	*/
	vPObject lineSystemObject;	/* object that holds the line system	*/
	vPGRenderable lineSystem;	/* line system							*/

	GLuint lineVertexBuffer;	/* gl VBO for line data					*/
	GLuint lineVertexArray;		/* gl VAO for drawArray call			*/
	vPGShader lineShader;		/* line shader							*/
} vGLineSystem, *vPGLineSystem;

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

	vHNDL exitCallbackList;		/* static list of exit callbacks */

	/* default shader metadata */
	vGDefaultShaderData defaultShaderData;

	/* default shaders */
	vGDefaultShaders defaultShaders;

	/* camera transform */
	vTransform cameraTransform;

	/* line system */
	vGLineSystem lineSystem;
} _vGInternals, *_vPGInternals;

_vGInternals _vgfx;	/* INSTANCE */


#endif
