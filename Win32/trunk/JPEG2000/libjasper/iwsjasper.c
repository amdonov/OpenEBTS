// iwsjasper.c: two simple functions for doing in-memory jp2/bmp conversion
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <io.h>
#include <tchar.h>
#include <jasper/jasper.h>
#include "iwsjasper.h"


bool JasperBMPtoJP2(char *pBMP, long lLenBMP, HGLOBAL *phJP2, float fRate)
{
	bool			bRet = false;
	jas_image_t		*image = NULL;
	jas_stream_t	*streamin = NULL;
	jas_stream_t	*streamout = NULL;
	char			szOptions[32];
	char			*pJP2 = NULL;
	long			lLenJP2;

	if (phJP2 == NULL) {
		goto done;
	} else {
		*phJP2 = NULL;
	}

	if (jas_init()) goto done;

	streamin = jas_stream_memopen(pBMP, lLenBMP);

	// Decode input (-1 means determine format)
	if (!(image = jas_image_decode(streamin, -1, NULL))) {
		goto done;
	}

	streamout = jas_stream_memopen(NULL, 0);

	// Encode output
	sprintf(szOptions, "rate=%f", fRate);
	if (jas_image_encode(image, streamout, jas_image_strtofmt("jp2"), szOptions)) {
		goto done;
	}
	jas_stream_flush(streamout);

	lLenJP2 = jas_stream_getrwcount(streamout);	//read-write count, aka "length"

	*phJP2 = GlobalAlloc(GPTR, lLenJP2);
	pJP2 = (char*)GlobalLock(*phJP2);

	jas_stream_rewind(streamout);
	if (jas_stream_read(streamout, pJP2, lLenJP2) != lLenJP2) {
		GlobalUnlock(*phJP2);
		GlobalFree(*phJP2);
		*phJP2 = NULL;
		goto done;
	}

	bRet = true;

done:
	if (streamin) jas_stream_close(streamin);
	if (streamout) jas_stream_close(streamout);
	if (image) jas_image_destroy(image);
	jas_cleanup();

	return bRet;
}

bool JasperJP2toBMP(char *pJP2, long lLenJP2, HGLOBAL *phBMP)
{
	bool			bRet = false;
	jas_image_t		*image = NULL;
	jas_stream_t	*streamin = NULL;
	jas_stream_t	*streamout = NULL;
	char			*pBMP = NULL;
	long			lLenBMP;

	if (phBMP == NULL) {
		goto done;
	} else {
		*phBMP = NULL;
	}

	if (jas_init()) goto done;

	streamin = jas_stream_memopen(pJP2, lLenJP2);

	// Decode input (-1 means determine format)
	if (!(image = jas_image_decode(streamin, -1, NULL))) {
		goto done;
	}

	streamout = jas_stream_memopen(NULL, 0);

	// Encode output
	if (jas_image_encode(image, streamout, jas_image_strtofmt("bmp"), NULL)) {
		goto done;
	}
	jas_stream_flush(streamout);

	lLenBMP = jas_stream_getrwcount(streamout);	//read-write count, aka "length"

	*phBMP = GlobalAlloc(GPTR, lLenBMP);
	pBMP = (char*)GlobalLock(*phBMP);

	jas_stream_rewind(streamout);
	if (jas_stream_read(streamout, pBMP, lLenBMP) != lLenBMP) {
		GlobalUnlock(*phBMP);
		GlobalFree(*phBMP);
		*phBMP = NULL;
		goto done;
	}

	bRet = true;

done:
	if (streamin) jas_stream_close(streamin);
	if (streamout) jas_stream_close(streamout);
	if (image) jas_image_destroy(image);
	jas_cleanup();

	return bRet;
}
