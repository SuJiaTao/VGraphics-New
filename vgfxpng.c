
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
VGFXAPI vPBYTE vGParsePNGUncompressed(vPBYTE fileData, vUI32 width, vUI32 height)
{
	/* generate all fillable chunks				*/
	PGImageChunk chunks = vAllocZeroed(sizeof(GImageChunk) * PNG_PARSE_MAXCHUNKS);
	vUI32 chunkCount = 0;

	/* png header is 8 bytes, skip the header	*/
	vUI64 fileReadIndex = 8;

	/* idat data */
	vUI64  idatTotalBytes = 0;
	vPBYTE idatConcatenated = NULL;
	vUI64  idatWriteBytes = 0;
	vUI64  idatParseIndex = 0;

	/* parsed image data */
	vPBYTE imageDataParsed = vAllocZeroed(width * height * 4ULL);

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

		/* if end, break loop */
		if (strcmp(currentChunk->name, "IEND") == ZERO) break;

		/* if idat, increment idatbytecounter */
		if (strcmp(currentChunk->name, "IDAT") == ZERO) 
			idatTotalBytes += currentChunk->length;

		printf("read chunk %s with len %d\n", currentChunk->name, currentChunk->length);

		/* goto chunk data and copy */
		fileReadIndex += 4;
		currentChunk->data = vAlloc(currentChunk->length);
		printf("copying data from %x to %x\n", fileReadIndex, fileReadIndex + currentChunk->length);
		if (currentChunk->length != 0)
			vMemCopy(currentChunk->data, fileData + fileReadIndex, currentChunk->length);

		/* jump all length */
		fileReadIndex += currentChunk->length;

		/* skip CRC */
		fileReadIndex += 4;

		/* increment chunk count */
		chunkCount++;
	}

	/* concatenate all idat chunks */
	idatConcatenated = vAllocZeroed(idatTotalBytes);
	
	for (int i = 0; i < chunkCount; i++)
	{
		PGImageChunk currentChunk = chunks + i;

		/* skip if not IDAT */
		if (strcmp(currentChunk->name, "IDAT") != 0) continue;

		printf("concatenating idat %d with offset %d and len %d\n", i,
			idatWriteBytes, currentChunk->length);

		/* concatenate */
		vMemCopy(idatConcatenated + idatWriteBytes,
			currentChunk->data, currentChunk->length);
		idatWriteBytes += currentChunk->length;
	}

	/* free all chunk data */
	for (int i = 0; i < chunkCount; i++)
	{
		vFree(chunks[i].data);
	}
	vFree(chunks);

	/* parse idat chunk */
	for (int i = 0; i < height; i++)
	{
		/* invert Y */
		int heightActual = height - i - 1;

		/* copy data */
		vPBYTE parseBlockWritePtr = imageDataParsed + (heightActual * width * 4ULL);
		vPBYTE imageDataReadPtr   = idatConcatenated + idatParseIndex;
		vMemCopy(parseBlockWritePtr, imageDataReadPtr, width * 4ULL);

		/* image byte index is incremented by the width + 1 to account for	*/
		/* the extra filter byte. what a pain!								*/
		idatParseIndex = idatParseIndex + (width * 4ULL) + 1;
	}

	/* free idat concatenated */
	vFree(idatConcatenated);
	
	return imageDataParsed;
}
