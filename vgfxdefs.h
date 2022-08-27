
/* ========== <vgfxdefs.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* All macros, typedefs and structure defs for VGFX			*/

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


/* ========== MAGIC NUMBERS						==========	*/
#define RENDER_OBJECTS_MAX			0x400
#define VGFX_TERMINATE_TRYTIME_MSEC	0x400
#define VGFX_TERMINATE_TRIES_MAX	0xA

#define VGFX_ASPECT_RATIO			1.5f
#define VGFX_RESOLUTION_HEIGHT		0x80
#define VGFX_RESOLUTION_WIDTH		VGFX_RESOLUTION_HEIGHT * VGFX_ASPECT_RATIO

#define VGFX_FRAMERATE				25
#define VGFX_RENDER_SLEEP_MSEC		1000 / VGFX_FRAMERATE


/* ========== STRINGS							==========	*/
#define VGFX_WINDOW_CLASS_NAME		"VGFX Render Window"

#define VGFX_SHADER_VERTEX_NAME		"vgfx_vert.txt"
#define VGFX_SHADER_FRAGMENT_NAME	"vgfx_frag.txt"


/* ========== TYPE DEFS							==========	*/


/* ========== STRUCTURES						==========	*/
typedef struct v2V
{
	float x;
	float y;
} v2V, *vP2V, vVector2, *vPVector2;

typedef struct vT2
{
	v2V   position;
	float scale;
	float rotation;
} vT2, *vPT2, vTransform2, *vPTransform2;

typedef struct vRect
{
	float width;
	float height;
} vRect, *vPRect;

typedef struct vColor4
{
	float R;
	float G;
	float B;
	float A;
} vColor4, *vPColor4;

typedef struct vTexture
{
	vUI32 totalWidth;
	vUI32 totalHeight;

	vUI16 skinCount;	/* each texture can have multiple skins, similar to	*/
						/* kind of like frames of an animation.	each skin	*/
						/* is lined up sequentially along the x axis		*/
	vUI16 skinWidth;
	vUI16 skinHeight;

	GLuint glHandle;	/* handle to GPU side object of texture				*/
} vTexture, *vPTexture;

typedef struct vRenderObject
{
	vBOOL render;			/* if 0, object is not rendered				*/

	vColor4  tint;			/* all frags are multiplied by this value	*/
	vTexture texture;		/* texture to be used						*/
	vUI16	 skin;			/* current texture skin						*/
	vRect	 rectangle;		/* object bounding rectangle				*/

	vTransform2 transform;	/* object's spacial info					*/
} vRenderObject, *vPRenderObject;

typedef struct _vGFXInternals
{
	vBOOL initialized;
	vBOOL killThreadSignal;	/* on 1, renderthread will exit				*/
	vBOOL windowClosed;		/* on 1, window has been closed				*/

	HDC deviceContext;		/* client's device context					*/

	HWND renderWindow;
	LONG renderClientWidth;
	LONG renderClientHeight;

	HGLRC  renderContext;	/* openGL render context object				*/
	HANDLE renderThread;	/* handle to render thread					*/
	vHNDL  renderThreadLock;	/* render thread sync object			*/

	vUI64 renderFrameCount;	/* increments with every frame rendered		*/

	vPRenderObject frameObject;

	GLuint shaderProgram;
	GLuint vertexAttributes;		/* VAO object */

	GLuint framebuffer;				/* framebuffer to render to			*/
	GLuint framebufferTexture;		/* framebuffer texture object		*/
	GLuint framebufferDepth;		/* framebuffer depth componenet		*/
	vTransform2 cameraPosition;		/* camera offset					*/

	vHNDL  renderObjectBuffer;		/* objects to render				*/
	GLuint renderObjectBaseRect;	/* base rect and UV					*/

} _vGFXInternals, *_vPGFXInternals;

_vGFXInternals _vgfx; /* INSTANCE */

#endif
