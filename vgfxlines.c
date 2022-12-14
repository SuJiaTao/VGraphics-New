
/* ========== <vgfxlines.c>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfxlines.h"
#include <stdio.h>


/* ========== SHADER SOURCE						==========	*/
static const char* vGLineVertexSource =
"#version 460 core\n"
"\n"
"layout (location = 0) in vec2 v_position;\n"
"layout (location = 1) uniform vec4 v_color;\n"
"layout (location = 2) uniform mat4 v_projection;\n"
"layout (location = 3) uniform mat4 v_model;\n"
"\n"
"out vec4 f_color;\n"
"\n"
"void main()\n"
"{\n"
"\tgl_Position = v_projection * v_model * vec4(v_position, 0.0, 1.0);\t\t\n"
"\tf_color = v_color;\n"
"}";
static const char* vGLineFragmentSource = 
"#version 460 core\n"
"\n"
"in vec4 f_color;\n"
"out vec4 FragColor;\n"
"\n"
"void main()\n"
"{\n"
"\tFragColor = f_color;\n"
"\t/* dithering alogrithm */\n"
"\tif (FragColor.a <= 0.97)\n"
"\t{\n"
"\t\tif (FragColor.a <= 0.03) discard;\n"
"\t\tint x = int(gl_FragCoord.x);\n"
"\t\tint y = int(gl_FragCoord.y);\n"
"\n"
"\t\tif (FragColor.a < 0.5)\n"
"\t\t{\n"
"\t\t\tint discardInterval = int(1.0f / FragColor.a);\n"
"\t\t\tint step = x + (y * (discardInterval >> 1)) + (y * (discardInterval >> 3));\n"
"\t\t\tif (step % discardInterval != 0) discard;\n"
"\t\t}\n"
"\t\telse\n"
"\t\t{\n"
"\t\t\tint discardInterval = int(1.0f / (1.0f - FragColor.a));\n"
"\t\t\tint step = x + (y * (discardInterval >> 1)) + (y * (discardInterval >> 3));\n"
"\t\t\tif (step % discardInterval == 0) discard;\n"
"\t\t}\n"
"\t}\n"
"\n"
"\tFragColor.a = 1.0;\n"
"}";


/* ========== SHADER FUNCTIONS					==========	*/
void vGLineShaderRenderIterateFunc(vHNDL dbuffer, vPGLine line, vPTR input)
{
	/* apply uniform values */
	glUniform4fv(1, 1, &line->col);

	/* copy vertex data to GL buffer */
	glBindBuffer(GL_ARRAY_BUFFER, _vgfx.lineSystem.lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vPosition) * 2, 
		line, GL_DYNAMIC_DRAW);

	/* bind to default vertex array */
	glBindVertexArray(_vgfx.lineSystem.lineVertexArray);
	glEnableVertexAttribArray(0);

	/* disable textures and blending */
	glDisable(GL_TEXTURE);
	glDisable(GL_BLEND);

	/* draw line */
	glLineWidth(line->width);
	glDrawArrays(GL_LINES, 0, 2);
}

void vGLineShader_initFunc(vPGShader shader, vPTR shaderData, vPTR input)
{
	/* create vertex array */
	glGenVertexArrays(1, &_vgfx.lineSystem.lineVertexArray);
	glBindVertexArray(_vgfx.lineSystem.lineVertexArray);

	/* create line vertex buffer	*/
	glGenBuffers(1, &_vgfx.lineSystem.lineVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vgfx.lineSystem.lineVertexBuffer);

	/* setup vertex attribute */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
}

void vGLineShader_renderFunc(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData)
{
	/* projection matrix is already setup (refer to vgfxthread.c) */
	/* apply camera transforms									  */
	glMatrixMode(GL_PROJECTION);

	vTransform cameraTransform = vGCameraGetTransform();
	glScalef(cameraTransform.scale, cameraTransform.scale, 1.0f);
	glRotatef(-cameraTransform.rotation, 0.0f, 0.0f, 1.0f);
	glTranslatef(-cameraTransform.position.x, -cameraTransform.position.y,
		0.0f);

	/* clear modelview matrix */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, LINE_ZDEPTH);

	/* clear texture matrix */
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	/* retrieve all data from gl matrix stack */
	GLfloat projectionMatrix[0x10];
	GLfloat modelMatrix[0x10];
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);

	/* apply uniform values */
	glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix);

	vDBufferIterate(_vgfx.lineSystem.lineList,
		vGLineShaderRenderIterateFunc, NULL);
	
	/* clear line buffer */
	vDBufferClear(_vgfx.lineSystem.lineList);
}

void vGLineBufferInitFunc(vHNDL hndl, vPGLine element, vPGLine input)
{
	vMemCopy(element, input, sizeof(vGLine));
}

/* ========== INIT FUNCTION						==========	*/
VGFXAPI vBOOL vGLineSystemInit(void)
{
	if (_vgfx.lineSystem.isInit == TRUE) return FALSE;

	/* create line buffer */
	_vgfx.lineSystem.lineList = vCreateDBuffer("vGFX Line List",
		sizeof(vGLine) * 2, LINELIST_NODE_SIZE, vGLineBufferInitFunc, NULL);

	/* create line object */
	_vgfx.lineSystem.lineSystemObject = vCreateObject(NULL);

	/* initialize line shader */
	_vgfx.lineSystem.lineShader = 
		vGCreateShader(vGLineShader_initFunc, vGLineShader_renderFunc, NULL, NULL,
			vGLineVertexSource, vGLineFragmentSource, NULL);

	_vgfx.lineSystem.lineSystem = vGCreateRenderable(_vgfx.lineSystem.lineSystemObject,
		vCreateTransformF(0, 0, 0, 1.0), _vgfx.lineSystem.lineShader,
		NULL, vGCreateRectCentered(0.1f, 0.1f), NULL);

	return TRUE;
}


/* ========== LINE DRAWING FUNCTIONS			==========	*/
VGFXAPI vGLine vGCreateLine(vPosition p1, vPosition p2, vGColor c, float width)
{
	vGLine rLine;
	rLine.p1 = p1;
	rLine.p2 = p2;
	rLine.col = c;
	rLine.width = max(1.0f, width);;
	return rLine;
}

VGFXAPI vBOOL vGDrawLineV(vPosition p1, vPosition p2, vGColor c, float width)
{
	vGLine testLine = vGCreateLine(p1, p2, c, width);

	/* if alpha == 0, don't draw */
	if (testLine.col.A == 0) return FALSE;

	/* add copy of testline object */
	vDBufferAdd(_vgfx.lineSystem.lineList, &testLine);

	return TRUE;
}

VGFXAPI vBOOL vGDrawLineF(float p1x, float p1y, float p2x, float p2y, vGColor c,
	float width)
{
	return vGDrawLineV(vCreatePosition(p1x, p1y), 
		vCreatePosition(p2x, p2y), c, width);
}

VGFXAPI void vGDrawLines(vPGLine lineList, vUI16 count)
{
	for (int i = 0; i < count; i++)
	{
		vPGLine line = lineList + i;
		vGDrawLineV(line->p1, line->p2, line->col, line->width);
	}
}

VGFXAPI void vGDrawLinesConnected(vPPosition pVect, vUI16 count, vGColor c, float width)
{
	for (int i = 0; i < count; i++)
	{
		vPosition p1 = pVect[i + 0];
		vPosition p2 = pVect[(i + 1) % count];
		vGDrawLineV(p1, p2, c, width);
	}
}

VGFXAPI void vGDrawCross(vPosition pos, float radius, vGColor c, float width)
{
	vGDrawLineF(pos.x - radius, pos.y + radius, pos.x + radius, pos.y - radius,
		c, width);
	vGDrawLineF(pos.x + radius, pos.y + radius, pos.x - radius, pos.y - radius,
		c, width);
}