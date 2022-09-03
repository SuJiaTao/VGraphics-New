
/* ========== <vgfxrenderthread.h>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#define GLEW_STATIC
#include "glew.h"
#include <stdio.h>
#include "vgfxrenderthread.h"
#include "vgfxshaders.h"
#include "vgfxdrawobject.h"


/* ========== HELPER							==========	*/
static __forceinline vhDestroyWindow(HWND winHndl)
{
	/* free openGL objects */
	glDeleteFramebuffers(1, &_vgfx.framebuffer);
	glDeleteRenderbuffers(1, &_vgfx.framebufferDepth);
	glDeleteTextures(1, &_vgfx.framebufferTexture);

	/* free resources */
	wglDeleteContext(_vgfx.renderContext);
	ReleaseDC(winHndl, _vgfx.deviceContext);
	UnregisterClassA(VGFX_WINDOW_CLASS_NAME, NULL);
}

static __forceinline void vhInitializeShaderProgram(void)
{
	vLogInfo(__func__, "Initializing shaders.");
	vBOOL loadResult;
	GLuint vertexShader, fragmentShader;


	vLogInfo(__func__, "Compiling vertex shader.");

	/* compile vertex shader */
	vertexShader = vGFXCompileShader(GL_VERTEX_SHADER, vGFXGetVertexShaderSource());
	if (vertexShader == ZERO) { vCoreFatalError(__func__, "Vertex shader could not be compiled."); }

	vLogInfo(__func__, "Vertex shader compiled sucessfully.");

	vLogInfo(__func__, "Compiling fragment shader.");

	/* compile fragment shader */
	fragmentShader = vGFXCompileShader(GL_FRAGMENT_SHADER, vGFXGetFragmentShaderSource());
	if (fragmentShader == ZERO) { vCoreFatalError(__func__, "Fragment shader could not be compiled."); }

	vLogInfo(__func__, "Fragment shader compiled sucessfully.");

	/* create shader program and use */
	_vgfx.defaultShader = vGFXCreateShaderProgram(vertexShader, fragmentShader);
	if (_vgfx.defaultShader == ZERO) { vCoreFatalError(__func__, "Could not create shader program."); }
	glUseProgram(_vgfx.defaultShader);

	vLogInfo(__func__, "Shaders initialized sucessfully.");
	vDumpEntryBuffer();
}

static LRESULT CALLBACK vGFXWindowProc(HWND winHndl, UINT message, 
	WPARAM wParam, LPARAM lParam)
{
	PIXELFORMATDESCRIPTOR pixelFormat;
	
	switch (message)
	{
	
	/* WINDOW CREATION */
	case WM_CREATE:

		/* configure pixelFormat */
		vZeroMemory(&pixelFormat, sizeof(pixelFormat));
		pixelFormat.nSize    = sizeof(pixelFormat);
		pixelFormat.nVersion = 1;
		pixelFormat.dwFlags  = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
			PFD_DOUBLEBUFFER;

		pixelFormat.iPixelType   = PFD_TYPE_RGBA;
		pixelFormat.cColorBits   = 32;
		pixelFormat.cDepthBits   = 16;
		pixelFormat.cStencilBits = 16;

		/* get device context */
		_vgfx.deviceContext = GetDC(winHndl);

		/* set window's pixel format */
		int pfHandle = ChoosePixelFormat(_vgfx.deviceContext, &pixelFormat);
		SetPixelFormat(_vgfx.deviceContext, pfHandle, &pixelFormat);
		
		/* create render context object */
		_vgfx.renderContext = wglCreateContext(_vgfx.deviceContext);
		wglMakeCurrent(_vgfx.deviceContext, _vgfx.renderContext);

		break;


	/* WINDOW DESTRUCTION */
	case WM_DESTROY:

		/* call window destroy helper */
		vhDestroyWindow(winHndl);

		/* set window closed flag */
		_vgfx.windowClosed = TRUE;

		break;


	default:
		break;
	}

	/* exec default window proc */
	return DefWindowProcA(winHndl, message, wParam, lParam);
}

static __forceinline void vhUpdateWindow(void)
{
	/* remove top message and dispatch */
	MSG messageBuff;
	PeekMessageA(&messageBuff, NO_WINDOW, ZERO, ZERO, PM_REMOVE);
	DispatchMessageA(&messageBuff);
}

static __forceinline void vhInitializeFramebuffer(void)
{
	/* generate openGL objects  and bind */
	vLogInfo(__func__, "Generating framebuffer render objects.");
	glGenFramebuffers(1, &_vgfx.framebuffer);
	glGenTextures(1, &_vgfx.framebufferTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, _vgfx.framebuffer);
	glBindTexture(GL_TEXTURE_2D, _vgfx.framebufferTexture);

	/* set texture size */
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGB,
		VGFX_RESOLUTION_WIDTH, VGFX_RESOLUTION_HEIGHT, ZERO,
		GL_RGB, GL_UNSIGNED_BYTE, NULL);

	/* make fb texture filter to nearest */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	/* generate depth component and bind */
	vLogInfo(__func__, "Generating framebuffer depth object.");
	glGenRenderbuffers(1, &_vgfx.framebufferDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, _vgfx.framebufferDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
		VGFX_RESOLUTION_WIDTH, VGFX_RESOLUTION_HEIGHT);

	/* connect to framebuffer and enable depth testing */
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, _vgfx.framebufferDepth);
	glEnable(GL_DEPTH_TEST);

	/* connect texture to framebuffer */
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		_vgfx.framebufferTexture, NULL);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	GLenum framebufferCheck = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferCheck != GL_FRAMEBUFFER_COMPLETE)
	{
		vLogErrorFormatted(__func__, "Framebuffer incomplete! Framebuffer status: %d.", framebufferCheck);
		vCoreFatalError(__func__, "Framebuffer incomplete.");
	}

	vLogInfo(__func__, "Framebuffer initialized.");
}

static __forceinline void vhInitializeRenderWindow(void)
{
	/* setup window descriptor object */
	WNDCLASSA windowDescriptor;
	vZeroMemory(&windowDescriptor, sizeof(windowDescriptor));
	
	/* set callback and name */
	windowDescriptor.lpfnWndProc   = vGFXWindowProc;
	windowDescriptor.lpszClassName = VGFX_WINDOW_CLASS_NAME;

	/* register class */
	ATOM windowClass = RegisterClassA(&windowDescriptor);

	/* on failed, log and terminate */
	if (windowClass == INVALID_ATOM)
	{
		vLogError(__func__, "Could not create window class for render window. Terminating process.");
		vCoreFatalError(__func__, "Window class creation failed.");
		ExitThread(ERROR_INVALID_FUNCTION);
	}

	/* calculate window size */
	int screenWidth  = GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, GetDpiForSystem());
	int windowWidth  = screenWidth * 0.65f;
	int windowHeight = windowWidth / VGFX_ASPECT_RATIO;

	/* adjust window to fit actual size */
	RECT clientRect = { 0, 0, windowWidth, windowHeight };
	DWORD windowStyle = WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX | WS_THICKFRAME;
	AdjustWindowRectExForDpi(&clientRect, windowStyle, TRUE, windowStyle, GetDpiForSystem());
	int windowWidthActual  = clientRect.right  - clientRect.left;
	int windowHeightActual = clientRect.bottom - clientRect.top;

	/* create win32 window object */
	_vgfx.renderWindow = CreateWindowExA(NO_FLAGS, VGFX_WINDOW_CLASS_NAME, " ", windowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, windowWidthActual, windowHeightActual, NO_WINDOW,
		ZERO, ZERO, NULL);

	/* if window was not created log error */
	if (_vgfx.renderWindow == NULL)
	{
		vLogError(__func__, "Render window creation failed. Terminating process.");
		vCoreFatalError(__func__, "Render window creation failed.");
		ExitThread(ERROR_INVALID_FUNCTION);
	}

	vLogInfo(__func__, "Render window created.");
	vLogInfo(__func__, "Initializing GLEW.");

	/* check for errors. */
	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK)
	{
		vLogErrorFormatted(__func__, "GLEW failed with error: %s. Terminating process", 
			glewGetErrorString(glewStatus));
		vCoreFatalError(__func__, "GLEW failed to initialized.");
		ExitThread(ERROR_INVALID_FUNCTION);
	}

}

static __forceinline vhExecuteRenderJobs(void)
{
	for (vUI32 jobCount = 0; jobCount < RENDERJOBS_PER_CYCLE; jobCount++)
	{
		/* LOCK BUFFER */ vLock(_vgfx.jobBuffer.jobBufferLock);

		/* if all jobs are complete, unlock and return */
		if (_vgfx.jobBuffer.jobsLeftToExecute == 0)
		{
			vUnlock(_vgfx.jobBuffer.jobBufferLock);
			return;
		}

		/* get job to execute */
		_vgfx.jobBuffer.jobStartIndex %= RENDERJOBS_PER_CYCLE;
		vPRenderJob toExecute = _vgfx.jobBuffer.jobs + _vgfx.jobBuffer.jobStartIndex;

		/* execute job (if applicable) and the zero job memory */
		if (toExecute->job != NULL) toExecute->job(toExecute->data);
		vZeroMemory(toExecute, sizeof(vRenderJob));

		/* increment start index and decrement jobs left */
		_vgfx.jobBuffer.jobStartIndex++;
		_vgfx.jobBuffer.jobsLeftToExecute--;

		/* UNLOCK BUFFER*/ vUnlock(_vgfx.jobBuffer.jobBufferLock);
	}
}

static __forceinline vhRenderFrame(void)
{
	/* bind to default framebuffer and clear */
	glBindFramebuffer(GL_FRAMEBUFFER, ZERO);
	glClearColor(VGFX_FAILEDRENDER_COLOR);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	/* clear all */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-VGFX_ASPECT_RATIO, VGFX_ASPECT_RATIO, -1, 1, -1, 1);
	glViewport(0, 0, _vgfx.renderClientWidth, _vgfx.renderClientHeight);
	
	vGFXDrawRenderObject(_vgfx.defaultRenderBuffer, _vgfx.frameObject);
}

static void vhDefaultRenderAttributeSetup(vPRenderBehavior behavior,
	vPDefaultRenderAttribute attribute)
{
	/* setup shader pointer */
	attribute->shaderProgram = &_vgfx.defaultShader;

	/* initialize framebuffer */
	vhInitializeFramebuffer(attribute);

	/* generate VAO to store vertex attribute descriptors */
	glGenVertexArrays(1, &attribute->vertexAttributes);
	glBindVertexArray(attribute->vertexAttributes);

	if (attribute->vertexAttributes == ZERO)
	{
		vLogError(__func__, "Could not create vertex array object.");
		vCoreFatalError(__func__, "Failed to create vertex array object.");
	}

	/* generate base rect for render objects (normalized square) */
	float baseRect[4][2] = { { -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1} };

	glGenBuffers(1, &attribute->renderObjectBaseRect);
	glBindBuffer(GL_ARRAY_BUFFER, attribute->renderObjectBaseRect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(baseRect), baseRect, GL_STATIC_DRAW);

	if (attribute->renderObjectBaseRect == ZERO)
	{
		vLogError(__func__, "Could not create vertex buffer object.");
		vCoreFatalError(__func__, "Failed to create vertex buffer object.");
	}

	/* setup vertex attributes */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

static void vhDefaultRenderMethod(vPDefaultRenderAttribute renderAttribute,
	vPTR objectAttribute, vPRenderObject object, GLfloat projectionMatrix[0x10],
	GLfloat modelMatrix[0x10], GLfloat textureMatrix[0x10])
{
	/* if object is frame object, draw to default framebuffer */
	if (object == _vgfx.frameObject)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, ZERO);
	}

	/* bind to shader program */
	glUseProgram(*renderAttribute->shaderProgram);

	/* bind to vertex buffer and array */
	glBindBuffer(GL_ARRAY_BUFFER, renderAttribute->renderObjectBaseRect);
	glBindVertexArray(renderAttribute->vertexAttributes);

	/* setup unfiform values */
	glUniform4fv(1, 1, &object->tint);
	glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(4, 1, GL_FALSE, textureMatrix);

	/* draw rects */
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, object->texture->glHandle);
	glDrawArrays(GL_QUADS, 0, 4);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

static void vhInitializeDefaultRenderBuffer(void)
{
	/* create default render buffer */
	_vgfx.defaultRenderBuffer = vGFXCreateRenderBuffer(_vgfx.defaultShader,
		vhDefaultRenderMethod, sizeof(vDefaultRenderAttribute), vhDefaultRenderAttributeSetup,
		ZERO, DEFAULT_RENDER_BUFFER_SIZE);

	/* create frame object */
	_vgfx.frameObject = vGFXCreateRenderObject(_vgfx.defaultRenderBuffer,
		vGFXCreateRect(VGFX_ASPECT_RATIO, 1.0f), NULL);
	_vgfx.frameObject->render  = FALSE; /* make invisible */
	_vgfx.frameObject->texture = vBufferAdd(_vgfx.textureBuffer);
	
	/* set texture to framebuffer texture */
	_vgfx.frameObject->texture->glHandle = _vgfx.framebufferTexture;
}

/* ========== RENDER THREAD ENTRY POINT			==========	*/
VGFXAPI void vGFXRenderThreadProcess(void* input)
{
	vLogInfo(__func__, "VGFX RenderThread started.");

	/* make process DPI aware to avoid weird window scaling */
	SetProcessDPIAware();

	/* initialize render window */
	vhInitializeRenderWindow();

	/* load, compile and use shaders */
	vhInitializeShaderProgram();

	/* setup shader vertex arrays */
	vGFXSetupShaderVertexAttributes();

	/* render time related variables */
	ULONGLONG currentRenderTimeMsec = 0;
	ULONGLONG lastRenderTimeMsec    = 0;
	ULONGLONG nextRenderTimeMsec    = 0;

	vLogInfo(__func__, "VGFX Starting render loop.");
	vDumpEntryBuffer(); /* dump in case of crash */

	/* render loop */
	while (_vgfx.killThreadSignal == FALSE)
	{
		/* ensure render sleep time has passed */
		currentRenderTimeMsec = GetTickCount64();
		nextRenderTimeMsec = lastRenderTimeMsec + VGFX_RENDER_SLEEP_MSEC;
		if (currentRenderTimeMsec < nextRenderTimeMsec)
		{
			DWORD sleepTime = nextRenderTimeMsec - currentRenderTimeMsec;
			Sleep(sleepTime);
		}

		/* SYNC */ vLock(_vgfx.renderThreadLock);

		/* update window */
		vhUpdateWindow();

		/* ensure window aspect ratio is fixed and set window height and width vars */
		RECT windowRect, clientRect;
		GetWindowRect(_vgfx.renderWindow, &windowRect);
		LONG windowWidth = windowRect.right - windowRect.left;
		SetWindowPos(_vgfx.renderWindow, NO_WINDOW, 0, 0,
			windowWidth, windowWidth / VGFX_ASPECT_RATIO, SWP_NOMOVE);

		GetClientRect(_vgfx.renderWindow, &clientRect);
		_vgfx.renderClientWidth  = clientRect.right  - clientRect.left;
		_vgfx.renderClientHeight = clientRect.bottom - clientRect.top;

		/* switch to custom framebuffer, clear and setup projection */
		glBindFramebuffer(GL_FRAMEBUFFER, _vgfx.framebuffer);
		glClearColor(VGFX_CLEAR_COLOR);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(-VGFX_ASPECT_RATIO, VGFX_ASPECT_RATIO, -1, 1, -1, 1);
		glViewport(0, 0, VGFX_RESOLUTION_WIDTH, VGFX_RESOLUTION_HEIGHT);

		/* perform camera transform */
		glRotatef(_vgfx.cameraTransform.rotation, 0, 0, 1.0f);
		glTranslatef(-_vgfx.cameraTransform.position.x, -_vgfx.cameraTransform.position.y, 0.0f);
		glScalef(_vgfx.cameraTransform.scale, _vgfx.cameraTransform.scale, 1.0f);

		/* execute render jobs */
		vhExecuteRenderJobs();

		/* draw all objects */
		vGFXDrawRenderObjects();

		/* switch to default framebuffer, and render frame */
		vhRenderFrame();

		/* swap buffers */
		SwapBuffers(_vgfx.deviceContext);

		/* increment render count */
		_vgfx.renderFrameCount++;

		/* UNSYNC */ vUnlock(_vgfx.renderThreadLock);
	}

	vLogInfo(__func__, "VGFX RenderThread recieved kill signal.");

	/* send destroy message to render window */
	DestroyWindow(_vgfx.renderWindow); 

	vLogInfo(__func__, "VGFX RenderThread exiting.");

	/* exit thread */
	ExitThread(ERROR_SUCCESS);
}