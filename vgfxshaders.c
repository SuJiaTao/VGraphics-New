
/* ========== <vgfxshaders.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfxshaders.h"


/* ========== ERROR SHADER						==========	*/
static vPCHAR vGShader_errVert =
	"#version 460 core\n"
	"\n"
	"layout (location = 0) in vec2 v_position;\n"
	"layout (location = 1) uniform vec4 v_color;\n"
	"layout (location = 2) uniform mat4 v_projection;\n"
	"layout (location = 3) uniform mat4 v_model;\n"
	"layout (location = 4) uniform mat4 v_texture;\n"
	"\n"
	"out vec2 f_textureUV;\n"
	"\n";

static vPCHAR vGShader_errFrag =
	"#version 460 core\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tfloat x = float(int(gl_FragCoord.x) >> 4);\n"
	"\tfloat y = float(int(gl_FragCoord.y) >> 4);\n"
	"\n"
	"\tif (mod(x + y, 2.0) == 0.0)\n"
	"\t{\n"
	"\t\tgl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
	"\t}\n"
	"\telse\n"
	"\t{\n"
	"\t\tgl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
	"\t}\n"
	"}";

vPCHAR vGShader_errRenderGetVert(void) { return vGShader_errVert; }
vPCHAR vGShader_errRenderGetFrag(void) { return vGShader_errFrag; }

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

	/* setup skin (if it exists) */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (renderData->skin != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, renderData->skin->glHandle);
		float textureSkinZoomScale = 1.0f / (float)(renderData->skin->skinCount + 1);
		glTranslatef(renderData->renderSkin * textureSkinZoomScale, 0.0f, 0.0f);
		glScalef(textureSkinZoomScale, 1.0f, 1.0f);
	}
	else
	{
		/* if no skin found, use flat skin texture */
		glBindTexture(GL_TEXTURE_2D, _vgfx.defaultShaderData.flatSkin->glHandle);
	}

	/* retrieve all data from gl matrix stack */
	GLfloat projectionMatrix[0x10];
	GLfloat modelMatrix[0x10];
	GLfloat textureMatrix[0x10];
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetFloatv(GL_TEXTURE_MATRIX, textureMatrix);

	/* apply uniform values */
	glUniform4fv(1, 1, &renderData->tint);
	glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(4, 1, GL_FALSE, textureMatrix);

	/* draw verts */
	glDrawArrays(GL_QUADS, 0, 4);


	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
}