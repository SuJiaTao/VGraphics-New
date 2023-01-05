
/* ========== <vgfxpng.c>						==========	*/
/* Bailey Jia-Tao Brown							2022		*/

/* ========== INCLUDES							==========	*/
#include "vgfxpng.h"
#include <stdlib.h>
#include <stdio.h>


/* ========== INTERNAL STRUCTS					==========	*/
typedef struct GImageChunk
{
	vUI32 length;
	vCHAR name[5];
	vPTR data;
} GImageChunk, *PGImageChunk;


/* ========== HELPER							==========	*/


/* ========== FUNCTIONS							==========	*/
VGFXAPI vPBYTE vGParsePNGUncompressed(vPBYTE fileData)
{
	/* generate all fillable chunks				*/
	PGImageChunk chunks = vAllocZeroed(sizeof(GImageChunk) * PNG_PARSE_MAXCHUNKS);
	vUI32 chunkCount = 0;

	/* png header is 8 bytes, skip the header	*/
	vUI64 fileReadIndex = 8;

	/* parse all chunks */
	while (TRUE)
	{
		PGImageChunk currentChunk = chunks + chunkCount;

		printf("parsing chunk %d\n", chunkCount);

		/* get chunk len */
		vMemCopy(&currentChunk->length, fileData + fileReadIndex, 
			sizeof(currentChunk->length));
		currentChunk->length = _byteswap_ulong(currentChunk->length); /* swap endian */

		/* get chunk type */
		fileReadIndex += 4;
		vMemCopy(currentChunk->name, fileData + fileReadIndex, 4);

		printf("found chunk %s with len %d\n", currentChunk->name, currentChunk->length);

		/* jump all length */
		fileReadIndex += currentChunk->length + 4ULL;

		if (strcmp(currentChunk->name, "IEND") == ZERO) break;

		/* skip CRC */
		fileReadIndex += 4;

		printf("current fileindex: %x\n", fileReadIndex);

		/* increment chunk count */
		chunkCount++;
	}

	vFree(chunks);
}
