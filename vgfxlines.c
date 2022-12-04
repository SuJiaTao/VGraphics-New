
/* ========== <vgfxlines.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Line drawing for debugging (not efficient)				*/


/* ========== INCLUDES							==========	*/
#include "glew.h"
#include "vgfxlines.h"


/* ========== SHADER SOURCE						==========	*/
static const char* vGLineVertexSource =
"#version 460 core\n"
"\n"
"layout (location = 0) in vec2 v_position;\n"
"layout (location = 1) uniform vec4 v_color;\n"
"layout (location = 2) uniform mat4 v_projection;\n"
"layout (location = 3) uniform mat4 v_model;\n"
"layout (location = 4) uniform mat4 v_texture;\n"
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
"}";


/* ========== LINE COLLISION FUNCTION			==========	*/
void vGLineListCollisionIterateFunc(vHNDL dbuffer, vPGLine line, vPGLine input)
{
	/* if line already exists, mark for skip */
	if (*(vPUI64)&line->p1 == *(vPUI64)&input->p1 &&
		*(vPUI64)&line->p2 == *(vPUI64)&input->p2) input->col.A = 0.0f;
}


/* ========== SHADER FUNCTIONS					==========	*/
void vGLineShaderRenderIterateFunc(vHNDL dbuffer, vPGLine line, vPTR input)
{
	/* apply uniform values */
	glUniform4fv(1, 1, &line->col);

	/* copy vertex data to GL buffer */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vPosition) * 2, line, GL_DYNAMIC_DRAW);

	/* bind to default vertex array */
	glBindVertexArray(_vgfx.defaultShaderData.vertexAttribute);

	/* draw line */
	glLineWidth(line->width);
	glDrawArrays(GL_LINES, 0, 2);
}

void vGLineShader_initFunc(vPGShader shader, vPTR shaderData, vPTR input)
{
	/* create line vertex buffer	*/
	glGenBuffers(1, &_vgfx.lineSystem.lineVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _vgfx.lineSystem.lineVertexBuffer);

	/* setup buffer */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vPosition) * 2, NULL, GL_DYNAMIC_DRAW);

	/* setup vertex attribute */
	glBindVertexArray(_vgfx.defaultShaderData.vertexAttribute);
	glEnableVertexAttribArray(0);
}

void vGLineShader_renderFunc(vPGShader shader, vPTR unused,
	vPObject object, vPGRenderable renderData)
{
	/* projection matrix is already setup (refer to vgfxthread.c) */
	/* apply camera transforms									  */
	glMatrixMode(GL_PROJECTION);

	vTransform cameraTransform = vGCameraGetTransform();
	glRotatef(-cameraTransform.rotation, 0.0f, 0.0f, 1.0f);
	glTranslatef(cameraTransform.position.x, cameraTransform.position.y,
		0.0f);
	glScalef(cameraTransform.scale, cameraTransform.scale, 1.0f);

	/* clear modelview matrix */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* clear texture matrix */
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	/* retrieve all data from gl matrix stack */
	GLfloat projectionMatrix[0x10];
	GLfloat modelMatrix[0x10];
	GLfloat textureMatrix[0x10];
	glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
	glGetFloatv(GL_TEXTURE_MATRIX, textureMatrix);

	/* apply uniform values */
	glUniformMatrix4fv(2, 1, GL_FALSE, projectionMatrix);
	glUniformMatrix4fv(3, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(4, 1, GL_FALSE, textureMatrix);

	vDBufferIterate(_vgfx.lineSystem.lineList,
		vGLineShaderRenderIterateFunc, NULL);

	/* clear line buffer */
	vDBufferClear(_vgfx.lineSystem.lineList);
}

/* ========== INIT FUNCTION						==========	*/
VGFXAPI vBOOL vGLineSystemInit(void)
{
	if (_vgfx.lineSystem.isInit == TRUE) return FALSE;

	/* create line buffer */
	_vgfx.lineSystem.lineList = vCreateDBuffer("vGFX Line Buffer",
		sizeof(vGLine), LINELIST_NODE_SIZE, NULL, NULL);

	/* create line object */
	_vgfx.lineSystem.lineSystemObject = vCreateObject(NULL);

	/* initialize line shader */
	_vgfx.lineSystem.lineShader = 
		vGCreateShader(vGLineShader_initFunc, vGLineShader_renderFunc, NULL, NULL,
			vGLineVertexSource, vGLineFragmentSource, NULL);

	_vgfx.lineSystem.lineSystem = vGCreateRenderable(_vgfx.lineSystem.lineSystemObject,
		vCreateTransformF(0, 0, 0, 1.0), _vgfx.lineSystem.lineShader,
		NULL, vGCreateRectCentered(1.0f, 1.0f));

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

	/* check if line already exists */
	vDBufferIterate(_vgfx.lineSystem.lineList, vGLineListCollisionIterateFunc, &testLine);

	/* if alpha == 0, don't draw */
	if (testLine.col.A == 0) return FALSE;

	vGLock();
	vPGLine line = vDBufferAdd(_vgfx.lineSystem.lineList, NULL);
	*line = testLine;
	vGUnlock();

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