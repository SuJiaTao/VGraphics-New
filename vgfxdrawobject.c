/* ========== <vgfxdrawobject.c>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include <gl/GL.h>
#include "vgfxdrawobject.h"
#include <stdio.h>


/* ========== HELPER					==========	*/
static void vhDrawRenderObjectCallback(vHNDL buffer, vUI16 index, vPRenderObject element)
{
	/* check if to skip render */
	if (element->render == FALSE) return;

	vGFXDrawRenderObject(element);
}


/* ========== OBJECT DRAWING					==========	*/
VGFXAPI void vGFXDrawRenderObject(vPRenderObject object)
{
	/* setup shader data */
	glBindBuffer(GL_ARRAY_BUFFER, _vgfx.renderObjectBaseRect);
	glBindVertexArray(_vgfx.vertexAttributes);
	glUseProgram(_vgfx.shaderProgram);

	/* apply transformations */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(object->transform.position.x, object->transform.position.y, 
		(float)object->layer / 255.0f);
	glRotatef(object->transform.rotation, 0.0f, 0.0f, 1.0f);
	glScalef(object->transform.scale * object->rectangle.width,
			 object->transform.scale * object->rectangle.height, 1.0f);

	/* setup texture skin */
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	float textureSkinZoomScale = 1.0f / (float)(object->texture->skinCount + 1);
	glTranslatef(object->skin * textureSkinZoomScale, 0.0f, 0.0f);
	glScalef(textureSkinZoomScale, 1.0f, 1.0f);

	/* setup shader uniform values */
	GLfloat projectionMatrix[0x10];
	GLfloat modelMatrix[0x10];
	GLfloat textureMatrix[0x10];
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX , modelMatrix);
	glGetFloatv(GL_TEXTURE_MATRIX   , textureMatrix);
	
	glUniform4fv(1, 1, &object->tint);
	glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(4, 1, GL_FALSE, textureMatrix);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glBindTexture(GL_TEXTURE_2D, object->texture->glHandle);
	glDrawArrays(GL_QUADS, 0, 4);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}

VGFXAPI void vGFXDrawRenderObjects(void)
{
	vBufferIterate(_vgfx.renderObjectBuffer, vhDrawRenderObjectCallback);
}