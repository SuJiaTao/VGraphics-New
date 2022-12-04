
/* ========== <vgfxlines.h>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/
/* Line drawing for debugging (not efficient)				*/

#ifndef _VGFX_LINES_INCLUDE_
#define _VGFX_LINES_INCLUDE_ 

/* ========== INCLUDES							==========	*/
#include "vgfx.h"


/* ========== INIT FUNCTION						==========	*/
VGFXAPI vBOOL vGLineSystemInit(void);


/* ========== LINE DRAWING FUNCTIONS			==========	*/
VGFXAPI vGLine vGCreateLine(vPosition p1, vPosition p2, vGColor c, float width);
VGFXAPI vBOOL vGDrawLineV(vPosition p1, vPosition p2, vGColor c, float width);
VGFXAPI vBOOL vGDrawLineF(float p1x, float p1y, float p2x, float p2y, vGColor c,
	float width);
VGFXAPI void vGDrawLines(vPGLine lineList, vUI16 count);
VGFXAPI void vGDrawLinesConnected(vPPosition pVect, vUI16 count, vGColor c, float width);


#endif
