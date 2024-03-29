
/* ========== <vgfxthread.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/

/* ========== INCLUDES							==========	*/
#define GLEW_STATIC

#include "glew.h"
#include "vgfx.h"
#include "vgfxthread.h"
#include "vgfxshaders.h"
#include <intrin.h>
#include <stdio.h>


/* ========== HELPER							==========	*/
static void vhGThreadValidateMemory(void)
{
	vPTR ptr = vAlloc(0x100);
	vFree(ptr);
}

static GLuint vhGCompileShader(GLenum shaderType, vPCHAR source)
{
	vLogInfo(__func__, "Started Compiling Shader.");

	/* if source is NULL, skip */
	if (source == NULL)
	{
		vLogError(__func__, "Tried to create shader with NULL src. Aborting.");
		return ZERO;
	}

	/* create empty shader object */
	GLuint shaderID = glCreateShader(shaderType);

	/* assign source and compile */
	vI32 srcLength = strlen(source);
	vLogInfoFormatted(__func__, "Passing source of length %d to openGL.",
		srcLength);
	glShaderSource(shaderID, 1, &source, &srcLength);
	glCompileShader(shaderID);

	/* ensure shader compiled properly */
	GLint compileStatus = GL_FALSE;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileStatus);

	/* if shader failed to compile, dump to file */
	if (compileStatus == GL_FALSE)
	{
		vLogError(__func__, "Shader failed to compile.");

		GLsizei writeLength; /* unused */

		/* get error message */
		GLchar* errorBuff = vAllocZeroed(BUFF_MASSIVE);
		glGetShaderInfoLog(shaderID, BUFF_MASSIVE, &writeLength, errorBuff);

		/* create err file name */
		vPCHAR errfileName = vAllocZeroed(BUFF_MEDIUM);
		sprintf_s(errfileName, BUFF_MEDIUM, "shader err log %I64X.txt", GetTickCount64());
		vLogErrorFormatted(__func__, "Dumping shader error to file: '%s'.", errfileName);

		/* create dump to file */
		HANDLE fHandle = vFileCreate(errfileName);
		vFileWrite(fHandle, ZERO, BUFF_MASSIVE, errorBuff);
		vFileClose(fHandle);

		/* free all memory */
		vFree(errorBuff);
		vFree(errfileName);

		vCoreFatalError(__func__, 
			"Failed to compiled shader. Please check shader error logs.");
		return ZERO;
	}

	vDumpEntryBuffer();
	return shaderID;
}

static void vhGExitCallbackList_iterateFunc(vHNDL buffer, vUI16 index,
	vPFGEXITCALLBACK* callbackPtr, vPTR input)
{
	vPFGEXITCALLBACK func = *callbackPtr;
	func();
}

static GLuint vhGCreateProgram(GLuint vert, GLuint frag)
{
	vLogInfo(__func__, "Creating shader program.");

	/* if either are ZERO, abort */
	if (vert == ZERO || frag == ZERO)
	{
		vLogError(__func__, "Vert and Frag shaders missing. Aborting.");
		return ZERO;
	}

	/* create new program and link vert & frag to it */
	GLuint shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vert);
	glAttachShader(shaderProgramID, frag);
	glLinkProgram(shaderProgramID);

	/* check linking status */
	GLuint linkStatus;
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &linkStatus);

	/* log linking failures */
	if (linkStatus == FALSE)
	{
		vLogError(__func__, "Shader program failed to link.");
		vCHAR errorBuffer[BUFF_LARGE];
		glGetProgramInfoLog(shaderProgramID, sizeof(errorBuffer), NULL, errorBuffer);
		vLogErrorFormatted(__func__, "Link error msg: '%s'.", errorBuffer);

		vCoreFatalError(__func__, "Failed to link shader program. Check logfile for details.");
		return ZERO;
	}

	vLogInfo(__func__, "Shader program created.");
	return shaderProgramID;
}

static void vhGSetupMissingTexture(void)
{
	const int missingTextureSize = 0x10;

	/* allocate memory for missing texture data */
	SIZE_T allocSize = 
		sizeof(vBYTE) * missingTextureSize * missingTextureSize * 4;
	vPBYTE missingTextureBytes = vAllocZeroed(allocSize);

	/* generate missing texture pattern */
	for (int x = 0; x < missingTextureSize; x++)
	{
		for (int y = 0; y < missingTextureSize; y++)
		{
			/* diagonal walk */
			int offsetY = x * missingTextureSize * 4;
			int offsetX = y * 4;

			/* get color component of current pixel */
			vBYTE* pixelColor = missingTextureBytes + offsetX + offsetY;
			vBYTE  patternColor[4] = { 255, 0, 255, 255 };

			/* checker pattern */
			if ((x + y) % 2 == 0)
			{
				vMemCopy(pixelColor, patternColor, sizeof(patternColor));
			}
			pixelColor[3] = 255;
		}
		
	}

	/* pass byte data to openGL */
	glGenTextures(1, &_vgfx.defaultShaderData.missingTexture);
	glBindTexture(GL_TEXTURE_2D, _vgfx.defaultShaderData.missingTexture);
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGBA, 
		missingTextureSize, missingTextureSize, ZERO, GL_RGBA,
		GL_UNSIGNED_BYTE, missingTextureBytes);

	vLogInfoFormatted(__func__, "Generated default texture object. OpenGL error: %d.",
		glGetError());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* forced linear filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	vFree(missingTextureBytes);
}

static void vhGSetupDefaultShaderData(void)
{
	vPGDefaultShaderData shaderData = &_vgfx.defaultShaderData;

	/* initialize VAO for basic rect */
	glGenVertexArrays(1, &shaderData->vertexAttribute);
	glBindVertexArray(shaderData->vertexAttribute);

	if (shaderData->vertexAttribute == ZERO)
	{
		vLogError(__func__, "Could not create vertex array object.");
		vCoreFatalError(__func__, "Failed to create vertex array object.");
	}

	/* create base rectangle (bottom left centered) */
	float baseRect[4][2] = { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0} };
	glGenBuffers(1, &shaderData->baseRect);
	glBindBuffer(GL_ARRAY_BUFFER, shaderData->baseRect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(baseRect), baseRect, GL_STATIC_DRAW);

	if (shaderData->baseRect == ZERO)
	{
		vLogError(__func__, "Could not create vertex buffer object.");
		vCoreFatalError(__func__, "Failed to create vertex buffer object.");
	}

	/* setup vertex attributes */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	/* setup missing texture object */
	vhGSetupMissingTexture();
}


/* ========== WIN32 WINDOW PROC CALLBACK		==========	*/
static LRESULT CALLBACK vGWindowProc(HWND window, UINT message,
	WPARAM wparam, LPARAM lparam)
{
	PIXELFORMATDESCRIPTOR pixelFormat;
	vZeroMemory(&pixelFormat, sizeof(PIXELFORMATDESCRIPTOR));

	PMINMAXINFO sizeLimitInfoOut;

	switch (message)
	{
	/* WINDOW SIZE LIMIT CALLBACK */
	case WM_GETMINMAXINFO:

		sizeLimitInfoOut = (PMINMAXINFO)lparam;
		sizeLimitInfoOut->ptMinTrackSize.x = WINDOW_WIDTH_MIN;
		sizeLimitInfoOut->ptMinTrackSize.y = WINDOW_HEIGHT_MIN;
		break;

	/* CREATION CALLBACK */
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

		/* bring window to top */
		if (BringWindowToTop(window) == FALSE)
		{
			vLogWarning(__func__, "Unable to bring window to top upon initialization.");
		}

		break;

	case WM_CLOSE:

		/* ignore close and leave it up to the user */
		vGLock();
		_vgfx.shouldExit = TRUE;
		vGUnlock();
		return TRUE;

	/* FINAL EXIT CALLBACK */
	case VGFX_EXIT_SIGNAL:
	case WM_DESTROY:

		/* run destruction callbacks */
		vBufferIterate(_vgfx.exitCallbackList, vhGExitCallbackList_iterateFunc, NULL);

		/* free DC */
		wglDeleteContext(_vgfx.window.renderingContext);
		ReleaseDC(_vgfx.window.window, _vgfx.window.deviceContext);
		UnregisterClassA(VGFX_WINDOW_CLASS_NAME, NULL);

		/* delete thread */
		vDestroyWorker(_vgfx.workerThread);

		break;
	}

	/* call default window proc (as defined by win32) */
	return DefWindowProcA(window, message, wparam, lparam);
}


/* ========== RENDERABLE DRAW FUNCTION			==========	*/
void vGRenderableListIterateDrawFunc(vHNDL dbHndl, vPGRenderable* element,
	vPTR input)
{
	/* pointer to element is passed to this function		*/
	/* element itself is a pointer to a renderable object	*/
	vPGRenderable renderable = *element;

	vPGShader shader = renderable->shader;

	/* call renderable's update func (if exists) */
	if (renderable->behavior.cycleFunc)
		renderable->behavior.cycleFunc(renderable);

	/* ensure shader doesn't exist, use err shader */
	if (shader == NULL) shader = _vgfx.defaultShaders.errShader;
	
	/* bind to shader handle */
	glUseProgram(shader->glProgramHandle);

	/* setup projection matrix */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	LONG width  = _vgfx.window.dimensions.right   - _vgfx.window.dimensions.left;
	LONG height = _vgfx.window.dimensions.bottom  - _vgfx.window.dimensions.top;
	float aspect = (float)width / (float)height;

	glViewport(0, 0, width, height);
	glOrtho(-aspect, aspect, -1.0, 1.0, ZNEAR_DEPTH, ZFAR_DEPTH);

	/* call shader functions */
	if (shader->renderFunc)
		shader->renderFunc(shader, shader->shaderDataPtr, renderable->objectPtr,
			renderable);
}


/* ========== RENDER THREAD FUNCTIONS			==========	*/
void vGRT_initFunc(vPWorker worker, vPTR workerData, 
	vPGInitializeData input)
{
	vLogInfo(__func__, "vGFX Render Thread initializing.");

	/* make process DPI aware */
	SetProcessDPIAware();

	/* set high self priority */
	vBOOL threadPriorityBool =
		SetThreadPriority(worker->thread, THREAD_PRIORITY_HIGHEST);

	/* setup window class and register */
	WNDCLASSA wClass;
	vZeroMemory(&wClass, sizeof(WNDCLASSA));
	wClass.lpfnWndProc   = vGWindowProc;
	wClass.lpszClassName = VGFX_WINDOW_CLASS_NAME;
	
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

	/* setup CLEAR color */
	glClearColor(VGFX_COLOR_0f, 1.0f);

	vLogInfo(__func__, "vGFX Render Thread initialized.");

	/* SETUP DEFAULT SHADER DATA */
	vhGSetupDefaultShaderData();

	/* FREE INPUT */
	vFree(input);

	/* DUMP ENTRY BUFFER */
	vDumpEntryBuffer();
}

void vGRT_exitFunc(vPWorker worker, vPTR workerData)
{
	
}

void vGRT_cycleFunc(vPWorker worker, vPTR workerData)
{
	vGLock();

	/* update window messaging system */
	MSG messageBuff;
	PeekMessageA(&messageBuff, NO_WINDOW, ZERO, ZERO, PM_REMOVE);
	DispatchMessageA(&messageBuff);

	/* update window state */
	GetClientRect(_vgfx.window.window, &_vgfx.window.dimensions);

	/* clear color and depth buffer */
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


/* ========== TASKABLE FUNCTIONS				==========	*/
void vGRT_createShaderTask(vPWorker worker, vPTR workerData, vPGRT_CShaderInput input)
{
	vLogInfo(__func__, "Creating Shader on Render Thread.");

	/* compile vertex shader */
	vLogInfo(__func__, "Compiling vertex shader.");
	GLuint vertShader = vhGCompileShader(GL_VERTEX_SHADER, input->vertexSrc);
	
	/* compile fragment shader */
	vLogInfo(__func__, "Compiling fragment shader.");
	GLuint fragShader = vhGCompileShader(GL_FRAGMENT_SHADER, input->fragSrc);

	/* link and assign to shader */
	input->shader->glVertHandle = vertShader;
	input->shader->glFragHandle = fragShader;
	input->shader->glProgramHandle = vhGCreateProgram(vertShader,
		fragShader);

	/* flag shaders for deletion (openGL will delete once program is destroyed) */
	glDeleteShader(input->shader->glVertHandle);
	glDeleteShader(input->shader->glFragHandle);

	/* call initialization */
	vPGShader shader = input->shader;
	if (shader->initFunc)
		shader->initFunc(shader, shader->shaderDataPtr, input->userInput);

	vLogInfo(__func__, "Shader created sucessfully.");

	/* FREE INPUT */
	vFree(input);
}

void vGRT_destroyShaderTask(vPWorker worker, vPTR workerData, vPTR input)
{
	vPGShader targetShader = input;

	/* call exit callback */
	if (targetShader->exitFunc)
		targetShader->exitFunc(targetShader, targetShader->shaderDataPtr);

	/* get GL to delete all objects */
	glDeleteProgram(targetShader->glProgramHandle);
	glDeleteShader(targetShader->glVertHandle);
	glDeleteShader(targetShader->glFragHandle);
}

void vGRT_createSkinTask(vPWorker worker, vPTR workerData, vPGRT_CSkinInput input)
{
	vPGSkin skin = input->skin;

	/* generate texture with alpha support */
	glGenTextures(1, &skin->glHandle);
	glBindTexture(GL_TEXTURE_2D, skin->glHandle);
	glTexImage2D(GL_TEXTURE_2D, ZERO, GL_RGBA, skin->width, skin->height, ZERO, GL_RGBA,
		GL_UNSIGNED_BYTE, input->byteData);

	/* apply wrap flags */
	if (skin->wrapped == TRUE)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}

	/* forced linear filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	vLogInfoFormatted(__func__, "Generated skin. OpenGL error: %d.",
		glGetError());

	/* FREE INPUT */
	vFree(input);
}

void vGRT_destroySkinTask(vPWorker worker, vPTR workerData, vPGSkin input)
{
	glDeleteTextures(1, &input->glHandle);
}

void vGRT_destroyWindowTask(vPWorker worker, vPTR workerData, vPTR unused)
{
	BOOL destroyResult = SendMessageA(_vgfx.window.window,
		VGFX_EXIT_SIGNAL, NULL, NULL);
}