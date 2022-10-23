
/* ========== <vgfxshaders.c>					==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfxshaders.h"
#include <stdio.h>


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
	"void main()\n"
	"{\n"
	"\tgl_Position = v_projection * v_model * vec4(v_position, 0.0, 1.0);\t\t\n"
	"}";

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
	"\t\tgl_FragColor = vec4(0.937, 0.733, 0.4, 1.0);\n"
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
static vPCHAR vGShader_rectVert =
	"#version 460 core\n"
	"\n"
	"layout (location = 0) in vec2 v_position;\n"
	"layout (location = 1) uniform vec4 v_color;\n"
	"layout (location = 2) uniform mat4 v_projection;\n"
	"layout (location = 3) uniform mat4 v_model;\n"
	"layout (location = 4) uniform mat4 v_texture;\n"
	"\n"
	"out vec2 f_textureUV;\n"
	"out vec4 f_colorMult;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tf_colorMult = v_color;\n"
	"\tf_textureUV = v_position;\n"
	"\tgl_Position = v_projection * v_model * vec4(v_position, 0.0, 1.0);\t\t\n"
	"}";

static vPCHAR vGShader_rectFrag =
	"#version 460 core\n"
	"\n"
	"in vec2 f_textureUV;\n"
	"in vec4 f_colorMult;\n"
	"\n"
	"uniform sampler2D f_texture;\n"
	"\n"
	"out vec4 FragColor;\n"
	"\n"
	"void main()\n"
	"{\n"
	"\tFragColor = texture(f_texture, f_textureUV) * f_colorMult;\n"
	"\n"
	"\t/* dithering alogrithm */\n"
	"\tif (FragColor.a <= 0.97)\n"
	"\t{\n"
	"\t\tif (FragColor.a <= 0.03) discard;\n"
	"\t\tint x = int(gl_FragCoord.x) >> 2;\n"
	"\t\tint y = int(gl_FragCoord.y) >> 2;\n"
	"\n"
	"\t\tint patternCount = 18;\n"
	"\n"
	"\t\tif (FragColor.a < 0.5)\n"
	"\t\t{\n"
	"\t\t\tint discardInterval = int((1.0 - (2.0 * FragColor.a)) * (patternCount - 1)) + 2;\n"
	"\t\t\tint step = x + (y * (discardInterval >> 1)) + (y * (discardInterval >> 3));\n"
	"\t\t\tif (step % discardInterval != 0) discard;\n"
	"\t\t}\n"
	"\t\telse\n"
	"\t\t{\n"
	"\t\t\tint discardInterval = int((FragColor.a - 0.5) * (patternCount - 1) * 2.0) + 2;\n"
	"\t\t\tint step = x + (y * (discardInterval >> 1)) + (y * (discardInterval >> 3));\n"
	"\t\t\tif (step % discardInterval == 0) discard;\n"
	"\t\t}\n"
	"\t}\n"
	"\n"
	"\tFragColor.a = 1.0;\n"
	"}";

vPCHAR vGShader_rectRenderGetVert(void) { return vGShader_rectVert; }
vPCHAR vGShader_rectRenderGetFrag(void) { return vGShader_rectFrag; }

void vGShader_rectRender(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData)
{
	vPGDefaultShaderData shaderData = &_vgfx.defaultShaderData;

	/* projection matrix is already setup (refer to vgfxthread.c) */
	/* setup modelview matrix									  */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* lastly move rect around */
	glTranslatef(object->transform.position.x, object->transform.position.y,
		(float)renderData->layer / 255.0f);
	glRotatef(object->transform.rotation, 0.0f, 0.0f, 1.0f);
	glScalef(object->transform.scale, object->transform.scale, 1.0f);

	/* firstly, adjust rect to be right size */
	glScalef(renderData->rect.right - renderData->rect.left,
		renderData->rect.top - renderData->rect.bottom, 1.0f);
	glTranslatef(renderData->rect.left, renderData->rect.bottom, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	/* setup skin (if it exists) */
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	/* default to use flat skin texture */
	glBindTexture(GL_TEXTURE_2D, _vgfx.defaultShaderData.missingTexture);

	/* set tint to be unused (unless texture is found) */
	vGColor renderTint = vGCreateColorF(1.0f, 1.0f, 1.0f, 1.0f);

	/* if skin exists, use that instead */
	if (renderData->skin != NULL)
	{
		renderTint = renderData->tint; /* set tint to match render data */

		/* bind texture and setup texture UV */
		glBindTexture(GL_TEXTURE_2D, renderData->skin->glHandle);
		float textureSkinZoomScale = 1.0f / (float)(renderData->skin->skinCount + 1);
		glTranslatef(renderData->renderSkin * textureSkinZoomScale, 0.0f, 0.0f);
		glScalef(textureSkinZoomScale, 1.0f, 1.0f);
	}

	/* bind to buffer and vertex array */
	glBindBuffer(GL_ARRAY_BUFFER, shaderData->baseRect);
	glBindVertexArray(shaderData->vertexAttribute);

	/* retrieve all data from gl matrix stack */
	GLfloat projectionMatrix[0x10];
	GLfloat modelMatrix[0x10];
	GLfloat textureMatrix[0x10];
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetFloatv(GL_TEXTURE_MATRIX, textureMatrix);

	/* apply uniform values */
	glUniform4fv(1, 1, &renderTint);
	glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(4, 1, GL_FALSE, textureMatrix);

	/* draw verts */
	glDrawArrays(GL_QUADS, 0, 4);

}