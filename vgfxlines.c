
/* ========== <vgfxlines.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Line drawing for debugging (not efficient)				*/


/* ========== INCLUDES							==========	*/
#include "vgfxlines.h"


/* ========== SHADER SOURCE						==========	*/
static const char* vGLineVertexSource = " ";
static const char* vGLineFragmentSource = " ";


/* ========== SHADER FUNCTIONS					==========	*/
void vGLineShaderRenderIterateFunc(vHNDL dbuffer, vPGLine line, vPTR input)
{
	
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


}

/* ========== INIT FUNCTION						==========	*/
vBOOL vGLineSystemInit(void)
{
	if (_vgfx.lineSystem.isInit == TRUE) return FALSE;

	/* create line buffer */
	_vgfx.lineSystem.lineList = vCreateDBuffer("vGFX Line Buffer",
		sizeof(vGLine), LINELIST_NODE_SIZE, NULL, NULL);

	/* create line object */
	_vgfx.lineSystem.lineSystemObject = vCreateObject(NULL);

	/* initialize line shader */
	_vgfx.lineSystem.lineShader = 
		vGCreateShader(NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	_vgfx.lineSystem.lineSystem = vGCreateRenderable(_vgfx.lineSystem.lineSystemObject,
		vCreateTransformF(0, 0, 0, 1.0), NULL, NULL, vGCreateRectCentered(1.0f, 1.0f));

	return TRUE;
}


/* ========== LINE DRAWING FUNCTIONS			==========	*/