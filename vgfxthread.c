
/* ========== <vgfxthread.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/

/* ========== INCLUDES							==========	*/
#define GLEW_STATIC

#include "glew.h"
#include "vgfx.h"
#include "vgfxthread.h"
#include <stdio.h>


/* ========== WIN32 WINDOW PROC CALLBACK		==========	*/
static LRESULT CALLBACK vGWindowProc(HWND window, UINT message,
	WPARAM wparam, LPARAM lparam)
{
	PIXELFORMATDESCRIPTOR pixelFormat;
	vZeroMemory(&pixelFormat, sizeof(PIXELFORMATDESCRIPTOR));

	switch (message)
	{
	case WM_CREATE:
		vLogInfo(__func__, "vGWindowProc recieved message WM_CREATE.");

		pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixelFormat.nVersion = 1;
		pixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
			PFD_DOUBLEBUFFER;

		pixelFormat.iPixelType   = PFD_TYPE_RGBA;
		pixelFormat.cColorBits   = 32;
		pixelFormat.cDepthBits   = 16;
		pixelFormat.cStencilBits = 16;

		/* get device context */
		_vgfx.window.deviceContext = GetDC(window);

		/* set window's pixel format */
		int pfHandle = ChoosePixelFormat(_vgfx.window.deviceContext, &pixelFormat);
		BOOL pixelFmtCheck = SetPixelFormat(_vgfx.window.deviceContext, pfHandle, &pixelFormat);
		if (pixelFmtCheck == FALSE)
		{
			vLogErrorFormatted(__func__, "SetPixelFormat returned FALSE.");
		}

		/* create render context object */
		_vgfx.window.renderingContext = wglCreateContext(_vgfx.window.deviceContext);
		BOOL wglCurrentCheck = wglMakeCurrent(_vgfx.window.deviceContext, 
			_vgfx.window.renderingContext);
		if (wglCurrentCheck == FALSE)
		{
			vLogErrorFormatted(__func__, "wglMakeCurrent returned FALSE");
		}

		break;
	}

	return DefWindowProcA(window, message, wparam, lparam);
}


/* ========== RENDERABLE DRAW FUNCTION			==========	*/
void vGRenderableListIterateDrawFunc(vHNDL dbHndl, vPGRenderable renderable,
	vPTR input)
{
	vPGShader shader = renderable->shader;

	/* bind to shader handle */
	glUseProgram(shader->glProgramHandle);

	/* call shader functions */
	if (shader->renderFunc)
		shader->renderFunc(shader, shader->shaderDataPtr, renderable->objectPtr,
			renderable);
}


/* ========== RENDER THREAD FUNCTIONS			==========	*/
void vGRenderThread_initFunc(vPWorker worker, vPTR workerData, 
	vPGInitializeData input)
{
	vLogInfo(__func__, "vGFX Render Thread initializing.");

	/* make process DPI aware */
	SetProcessDPIAware();
	
	/* setup window class and register */
	WNDCLASSA wClass;
	vZeroMemory(&wClass, sizeof(WNDCLASSA));
	wClass.lpfnWndProc   = vGWindowProc;
	wClass.lpszClassName = "vGFX Window Class";
	
	ATOM result = RegisterClassA(&wClass);
	if (result == INVALID_ATOM)
	{
		vLogError(__func__, "Could not register render window class.");
		vCoreFatalError(__func__, "Window class failed to register.");
	}

	/* setup window style */
	DWORD windowStyle = WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX | WS_THICKFRAME;

	/* get proper window rect */
	RECT clientRect = { 0, 0, input->windowWidth, input->windowHeight };
	AdjustWindowRectExForDpi(&clientRect, windowStyle, TRUE, windowStyle,
		GetDpiForSystem());
	DWORD windowWidthActual  = clientRect.right - clientRect.left;
	DWORD windowHeightActual = clientRect.bottom - clientRect.top;

	/* create window */
	_vgfx.window.window = CreateWindowExA(ZERO, "vGFX Window Class",
		input->windowName, windowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
		windowWidthActual, windowHeightActual, NO_WINDOW, ZERO, ZERO, NULL);
	vLogInfoFormatted(__func__, "Render Window Created as: %p.", _vgfx.window.window);

	/* ensure window was created */
	if (_vgfx.window.window == NO_WINDOW)
	{
		vLogError(__func__, "CreateWindowExA returned NULL. Window failed to create.");
		vCoreFatalError(__func__, "Failed to create render window.");
	}

	/* initialize GLEW */
	glewInit();

	vLogInfo(__func__, "vGFX Render Thread initialized.");
}

void vGRenderThread_exitFunc(vPWorker worker, vPTR workerData)
{

}

void vGRenderThread_cycleFunc(vPWorker worker, vPTR workerData)
{
	vGLock();

	/* update window messaging system */
	MSG messageBuff;
	PeekMessageA(&messageBuff, NO_WINDOW, ZERO, ZERO, PM_REMOVE);
	DispatchMessageA(&messageBuff);

	/* update window state */
	GetClientRect(_vgfx.window.window, &_vgfx.window.dimensions);

	/* setup viewport and clear */
	glViewport(0, 0, _vgfx.window.dimensions.left - _vgfx.window.dimensions.right,
		_vgfx.window.dimensions.top - _vgfx.window.dimensions.bottom);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* draw all renderables */
	vDBufferIterate(_vgfx.renderableList,
		vGRenderableListIterateDrawFunc, NULL);

	/* swap buffers */
	BOOL swapCheck = SwapBuffers(_vgfx.window.deviceContext);
	if (swapCheck == FALSE)
	{
		vLogWarning(__func__, "SwapBuffers returned FALSE. Frame did not render properly.");
	}

	vGUnlock();
}