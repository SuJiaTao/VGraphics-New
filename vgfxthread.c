
/* ========== <vgfxthread.h>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/

/* ========== INCLUDES							==========	*/
#include "vgfx.h"
#include "vgfxthread.h"


/* ========== WIN32 WINDOW PROC CALLBACK		==========	*/
static LRESULT CALLBACK vGWindowProc(HWND window, UINT message,
	WPARAM wparam, LPARAM lparam)
{

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

	/* ensure window was created */
}

void vGRenderThread_exitFunc(vPWorker worker, vPTR workerData)
{

}

void vGRenderThread_cycleFunc(vPWorker worker, vPTR workerData)
{

}