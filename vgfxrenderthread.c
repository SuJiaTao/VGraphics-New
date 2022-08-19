
/* ========== <vgfxrenderthread.h>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#define GLEW_STATIC
#include "glew.h"
#include "vgfxrenderthread.h"


/* ========== HELPER							==========	*/
static __forceinline vhDestroyWindow(HWND winHndl)
{
	ReleaseDC(winHndl, _vgfx.deviceContext);
	wglDeleteContext(_vgfx.renderContext);
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
	return DefWindowProcW(winHndl, message, wParam, lParam);
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
	glBindFramebuffer(GL_FRAMEBUFFER, _vgfx.framebuffer);
	glGenTextures(1, &_vgfx.framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, _vgfx.framebufferTexture);

	/* set texture size */
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGB4_EXT,
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

/* ========== RENDER THREAD ENTRY POINT			==========	*/
VGFXAPI void vGFXRenderThreadProcess(void* input)
{
	vLogInfo(__func__, "VGFX RenderThread started.");

	/* make process DPI aware to avoid weird window scaling */
	SetProcessDPIAware();

	/* initialize render window */
	vhInitializeRenderWindow();

	/* initialize framebuffer */
	vhInitializeFramebuffer();

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

		/* clear buffer */
		__glewBindFramebuffer(GL_FRAMEBUFFER, ZERO); /* bind to default framebuffer */
		glClear(GL_COLOR_BUFFER_BIT);

		/* swap buffers */
		SwapBuffers(_vgfx.deviceContext);

		/* increment render count */
		_vgfx.renderFrameCount++;

		/* update window */
		vhUpdateWindow();

		/* UNSYNC */ vUnlock(_vgfx.renderThreadLock);
	}

	vLogInfo(__func__, "VGFX RenderThread recieved kill signal.");

	/* send destroy message to render window */
	DestroyWindow(_vgfx.renderWindow); 

	vLogInfo(__func__, "VGFX RenderThread exiting.");

	/* exit thread */
	ExitThread(ERROR_SUCCESS);
}