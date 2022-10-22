
/* ========== <vgfxshaders.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfxshaders.h"


/* ========== RECT SHADER						==========	*/
static const char* rectShaderSource = " ";

void vGShader_rectInit(vPGShader shader, vPGS_rectData shaderData, vPTR input)
{
	/* initialize GLEW */
	glewInit();

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
}

void vGShader_rectRender(vPGShader shader, vPGS_rectData shaderData,
	vPObject object, vPGRenderable renderData)
{
	/* bind to buffer and vertex array */
	glBindBuffer(GL_ARRAY_BUFFER, shaderData->baseRect);
	glBindVertexArray(shaderData->vertexAttribute);


	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);


	/* projection matrix is already setup (refer to vgfxthread.c) */
	/* setup modelview matrix									  */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(object->transform.position.x, object->transform.position.y,
		(float)renderData->layer / 255.0f);
	glRotatef(object->transform.rotation, 0.0f, 0.0f, 1.0f);
	glScalef(object->transform.scale, object->transform.scale, 1.0f);

	/* setup skin */
	glBindTexture(GL_TEXTURE_2D, renderData->skin->glHandle);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	float textureSkinZoomScale = 1.0f / (float)(renderData->skin->skinCount + 1);
	glTranslatef(renderData->renderSkin * textureSkinZoomScale, 0.0f, 0.0f);
	glScalef(textureSkinZoomScale, 1.0f, 1.0f);

	/* draw verts */
	glDrawArrays(GL_QUADS, 0, 4);


	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

void vGShader_rectExit(vPGShader shader, vPTR shaderData);