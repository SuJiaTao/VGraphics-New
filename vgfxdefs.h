
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
#define RENDER_BEHAVIORS_MAX		0x40
#define RENDER_BUFFERS_MAX			0x100
#define TEXTURE_OBJECTS_MAX			0x200
#define DEFAULT_RENDER_BUFFER_SIZE	0x010
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

#define RENDERATTRIBUTE_SIZE_MIN	4

#define RENDERJOBS_PER_CYCLE		0x100
#define RENDERJOB_WAIT_MSEC			4

/* ========== STRINGS							==========	*/
#define VGFX_WINDOW_CLASS_NAME		"VGFX Render Window"


/* ========== TYPE DEFS							==========	*/
typedef void (*vGFXPFRenderJob)(vPTR data);
typedef void (*vGFXPFRenderAttributeSetup)(struct vRenderBehavior* behavior, vPTR renderAttribute);
typedef void (*vGFXPFRenderMethod)(vPTR renderAttribute, vPTR objectAttribute,
	struct vRenderObject* object, GLfloat projectionMatrix[0x10], GLfloat modelMatrix[0x10], 
	GLfloat textureMatrix[0x10]);

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
	vUI8  layer;		/* transform layer (0 bottom, 255 top)	*/
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

typedef struct vRenderBehavior
{
	GLuint			   shader;			/* gl Handle to shader object			*/
	vGFXPFRenderMethod renderMethod;	/* render logic for renderObjects		*/
	
	/* render attribute is a block of data which is associated with a given		*/
	/* render behavior and is the same to all render objects using the bhvr		*/
	vUI32 renderAttributeSizeBytes;
	vPTR  renderAttributePtr;

	/* this is called once when the render behavior is created to initialize	*/
	/* the data within the render attribute										*/
	vGFXPFRenderAttributeSetup renderAttributeSetup;

	/* object attribute is a block of data which is associated with each		*/
	/* render object and is passed to the rendermethod							*/
	vUI32 objectAttributeSizeBytes;	
} vRenderBehavior, *vPRenderBehavior;

typedef struct vRenderObject
{
	/* determines how the object is to be rendered */
	vPRenderBehavior renderBehavior;

	vBOOL render;			/* if 0, object is not rendered				*/
	
	vPTexture texture;		/* texture to be used						*/

	vColor4   tint;			/* all frags are multiplied by this value	*/
	vUI16	  skin;			/* current texture skin						*/
	vRect	  rectangle;	/* object bounding rectangle				*/

	vTransform2 transform;	/* object's spacial info					*/
} vRenderObject, *vPRenderObject;

typedef struct vRenderBuffer
{
	vPRenderBehavior renderBehavior;	/* how to draw each object	*/
	vHNDL objectBuffer;					/* object buffer			*/
} vRenderBuffer, *vPRenderBuffer;

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

typedef struct vDefaultRenderAttribute
{
	GLuint* shaderProgram;			/* ptr to shader prog hndl			*/
	GLuint  vertexAttributes;		/* VAO object						*/
	GLuint  renderObjectBaseRect;	/* base rect and UV					*/
} vDefaultRenderAttribute, *vPDefaultRenderAttribute;

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

	GLuint defaultShader;			/* default shader program			*/
	vPRenderBuffer defaultRenderBuffer;	/* default RenderBuffer			*/
	
	GLuint  framebuffer;			/* framebuffer to render to			*/
	GLuint  framebufferTexture;		/* framebuffer texture object		*/
	GLuint  framebufferDepth;		/* framebuffer depth componenet		*/
	vPRenderObject frameObject;		/* framebuffer render object		*/

	vTransform2 cameraTransform;	/* camera transform					*/

	/* buffer that holds all textures									*/
	vHNDL textureBuffer;			

	/* buffer that holds all render behaviors							*/
	vHNDL renderBehaviorBuffer;

	/* buffer of buffers that hold render objects.						*/
	vHNDL renderBuffers;

	vRenderJobBuffer jobBuffer;		/* jobs to execute per render cycle */
} _vGFXInternals, *_vPGFXInternals;

_vGFXInternals _vgfx; /* INSTANCE */

#endif
