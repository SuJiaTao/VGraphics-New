
/* ========== <vgfxshaders.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfxshaders.h"


/* ========== ERROR SHADER						==========	*/
void vGShader_errRender(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData)
{
	/* CALLS RECT SETUP, SHADER PROGRAM IS DIFFERENT */
	vGShader_rectRender(shader, unused, object, renderData);
}


/* ========== RECT SHADER						==========	*/
void vGShader_rectRender(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData)
{
	vPGDefaultShaderData shaderData = &_vgfx.defaultShaderData;

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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float textureSkinZoomScale = 1.0f / (float)(renderData->skin->skinCount + 1);
	glTranslatef(renderData->renderSkin * textureSkinZoomScale, 0.0f, 0.0f);
	glScalef(textureSkinZoomScale, 1.0f, 1.0f);

	/* draw verts */
	glDrawArrays(GL_QUADS, 0, 4);


	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}