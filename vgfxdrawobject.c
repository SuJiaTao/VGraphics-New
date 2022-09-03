/* ========== <vgfxdrawobject.c>				==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include <gl/GL.h>
#include "vgfxdrawobject.h"
#include <stdio.h>


/* ========== HELPER					==========	*/
static void vhDrawRenderObjectCallback(vPRenderBuffer buffer, vUI16 index, vPRenderObject element)
{
	/* check if to skip render */
	if (element->render == FALSE) return;

	vGFXDrawRenderObject(buffer, element);
}


/* ========== OBJECT DRAWING					==========	*/
VGFXAPI void vGFXDrawRenderObject(vPRenderBuffer buffer, vPRenderObject object)
{
	/* switch to using object's associated shader */
	glUseProgram(buffer->renderBehavior->shader);

	/* apply transformations */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(object->transform.position.x, object->transform.position.y, 
		(float)object->transform.layer / 255.0f);
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

	/* bind to framebuffer */
	glBindFramebuffer(GL_FRAMEBUFFER, _vgfx.framebuffer);

	/* call render method */
	vPBYTE objectAttributePtr = (vPBYTE)object + sizeof(vRenderObject);
	buffer->renderBehavior->renderMethod(buffer->renderBehavior->renderAttributePtr,
		objectAttributePtr, object, projectionMatrix, modelMatrix, textureMatrix);
}

VGFXAPI void vGFXDrawRenderObjects(void)
{
	vBufferIterate(_vgfx.renderBuffers, vhDrawRenderObjectCallback);
}