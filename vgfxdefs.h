
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
#define RENDER_OBJECTS_MAX			0x1000
#define TEXTURE_OBJECTS_MAX			0x100
#define VGFX_TERMINATE_TRYTIME_MSEC	0x400
#define VGFX_TERMINATE_TRIES_MAX	0xA

#define VGFX_SHADERSOURCE_SIZE_MAX	0x1000

#define VGFX_ASPECT_RATIO			1.5f
#define VGFX_RESOLUTION_HEIGHT		0x180
#define VGFX_RESOLUTION_WIDTH		VGFX_RESOLUTION_HEIGHT * VGFX_ASPECT_RATIO

#define VGFX_FRAMERATE				25
#define VGFX_RENDER_SLEEP_MSEC		1000 / VGFX_FRAMERATE

#define VGFX_CLEAR_COLOR			0.298f, 0.349f, 0.439f, 1.000f
#define VGFX_FAILEDRENDER_COLOR		1.0f, 0.0f, 1.0f, 1.0f

#define RENDERJOBS_PER_CYCLE		0x100
#define RENDERJOB_WAIT_MSEC			4

/* ========== STRINGS							==========	*/
#define VGFX_WINDOW_CLASS_NAME		"VGFX Render Window"

#define VGFX_SHADER_VERTEX_NAME		"vgfx_vert.txt"
#define VGFX_SHADER_FRAGMENT_NAME	"vgfx_frag.txt"


/* ========== TYPE DEFS							==========	*/
typedef void (*vGFXPFRenderJob)(vPTR data);

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

	GLuint glHandle;	/* handle to GPU side object of texture				*/
} vTexture, *vPTexture;

typedef struct vRenderObject
{
	vBOOL render;			/* if 0, object is not rendered				*/

	vColor4   tint;			/* all frags are multiplied by this value	*/
	vPTexture texture;		/* texture to be used						*/
	vUI16	  skin;			/* current texture skin						*/
	vRect	  rectangle;	/* object bounding rectangle				*/
	vUI8	  layer;		/* object layer (0 bottom, 255 top)			*/

	vTransform2 transform;	/* object's spacial info					*/
} vRenderObject, *vPRenderObject;

typedef struct vRenderJob
{
	vPTR  data;			/* arbitrary data pointer for job. must be cleaned	*/
						/* up by the job. can be NULL						*/

	vGFXPFRenderJob job;	/* function to execute on render thread			*/
} vRenderJob, *vPRenderJob;

typedef struct vRenderJobBuffer
{
	vHNDL jobBufferLock;				/* sync object								*/

	vRenderJob jobs[RENDERJOBS_PER_CYCLE];	/* jobs are executed in FIFO order			*/
	vUI32	   jobsLeftToExecute;		/* job count								*/
	vUI32	   jobStartIndex;			/* circular array head index				*/
} vRenderJobBuffer, *vPRenderJobBuffer;

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
	vTransform2 cameraTransform;	/* camera transform					*/

	vHNDL textureBuffer;			/* buffer that holds all textures	*/

	vHNDL  renderObjectBuffer;		/* objects to render				*/
	GLuint renderObjectBaseRect;	/* base rect and UV					*/

	vRenderJobBuffer jobBuffer;		/* jobs to execute per render cycle */
} _vGFXInternals, *_vPGFXInternals;

_vGFXInternals _vgfx; /* INSTANCE */

#endif
