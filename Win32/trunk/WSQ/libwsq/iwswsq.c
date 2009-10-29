// iwswsq.c: two simple functions for doing in-memory wsq/bmp conversion
//

// Important!!! Build with __i386__ flag to get little-endian in the output

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <io.h>
#include <tchar.h>
#include <windows.h>
#include "wsq.h"
#include "iwswsq.h"

int debug = 0; 

// To pass WSQ certification, we cannot have any comments
#define WSQCOMMENT ""


int NISTBMPtoWSQ(char *pBMP, long lLenBMP, HGLOBAL *phWSQ, float fRate)
// Input must be 8bpp BMP uncompressed. Since wsq_encode_mem wants a raw bitmap without
// the Gates padding and because BMPs are upside down, we copy over the bytes into a
// temporary array.
{
	int					iRet = 0;
	BITMAPFILEHEADER	*pbfh;
	BITMAPINFOHEADER	*pbih;
	int					iDPI;
	int					iRetWSQ;
	BYTE				*pRaw = NULL;
	BYTE				*pOut;
	int					iLenOut;
	void				*pWSQ;
	long				lBytesPerRowBMP;
	long				i;
	long				lLenPalette;
	BYTE				*pBMPPtr;
	BYTE				*pRawPtr;

	if (phWSQ == NULL || fRate < 0.04 || fRate > 4.0) {
		goto done;
	} else {
		*phWSQ = NULL;
	}

	// Make this work for both file and memory BMPs
	pbfh = (BITMAPFILEHEADER*)pBMP;
	if (pbfh->bfType == 0x4d42) { //"BM"
		pbih = (BITMAPINFOHEADER*)(pBMP + sizeof(BITMAPFILEHEADER));
	} else {
		pbih = (BITMAPINFOHEADER*)pBMP;
	}

	if (pbih->biCompression != BI_RGB) return 0;	// only uncompressed supported
	if (pbih->biBitCount != 8) return 0;			// only 8bpp supported

	iDPI = (int)((float)pbih->biXPelsPerMeter * 254 / 10000 + .5);
	// If not provided, assume 500. This is used mostly for fingerprints after all
	if (iDPI == 0) iDPI = 500;

	// Set pBMPPtr to where image bits start.
	lLenPalette = (pbih->biClrUsed == 0 ? 256 :  pbih->biClrUsed) * sizeof(RGBQUAD);
	pBMPPtr = (BYTE*)pbih + sizeof(BITMAPINFOHEADER) + lLenPalette;
	// Rows are 4-byte aligned...
	lBytesPerRowBMP = (pbih->biWidth + 3)/4*4;
	// Since the BMP is upside down, move the pointer to the last row
	pBMPPtr += lBytesPerRowBMP*(pbih->biHeight-1);

	pRaw = malloc(pbih->biWidth * pbih->biHeight);
	pRawPtr = pRaw;

	// We transfer each row into the new array, with 4-byte alignment
	// and with the rows reversed
	for (i=0; i<pbih->biHeight; i++) {
		memcpy(pRawPtr, pBMPPtr, pbih->biWidth);
		pRawPtr += pbih->biWidth;		// move down one raw row
		pBMPPtr -= lBytesPerRowBMP;		// move up one BMP row
	}

	iRetWSQ = wsq_encode_mem(&pOut, &iLenOut, fRate, pRaw, pbih->biWidth, pbih->biHeight, 8, iDPI, WSQCOMMENT);

	if (iRetWSQ == 0) {
		
		*phWSQ = GlobalAlloc(GPTR, iLenOut);
		pWSQ = GlobalLock(*phWSQ);
		memcpy(pWSQ, pOut, iLenOut);
		GlobalUnlock(*phWSQ);
		free(pOut); // allocated in wsq_encode_mem via malloc

		iRet = 1;
	}

done:
	if (pRaw != NULL) free(pRaw);

	return iRet;
}

int NISTWSQtoBMP(char *pWSQ, long lLenWSQ, HGLOBAL *phBMP)
{
	int					iRet = 0;
	int					iRetWSQ;
	BYTE				*pOut;
	int					iWidth;
	int					iHeight;
	int					iDepth;
	int					iDPI;
	int					iLossy;
	long				lBytesPerRowBMP;
	long				lLenBMP;
	BYTE				*pBMP;
	BYTE				*pBMPPtr;
	BYTE				*pRawPtr;
	BITMAPFILEHEADER	*pbfh;
	BITMAPINFOHEADER	*pbih;
	RGBQUAD				*pRGBQ;
	long				i;

	if (phBMP == NULL) {
		goto done;
	} else {
		*phBMP = NULL;
	}

	iRetWSQ = wsq_decode_mem(&pOut, &iWidth, &iHeight, &iDepth, &iDPI, &iLossy, pWSQ, lLenWSQ);

	if (iRetWSQ == 0) {

		// WSQ must be 8 bpp
		if (iDepth != 8) {
			free(pOut);  // allocated in wsq_decode_mem via malloc
			goto done;
		}

		// Form BMP (file version)

		// Rows are 4-byte aligned...
		lBytesPerRowBMP = (iWidth + 3)/4*4;

		lLenBMP = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + lBytesPerRowBMP*iHeight;

		*phBMP = GlobalAlloc(GPTR, lLenBMP);
		pBMP = (BYTE*)GlobalLock(*phBMP);

		pbfh = (BITMAPFILEHEADER*)pBMP;
		memset(pbfh, 0, sizeof(BITMAPFILEHEADER));
		pbfh->bfType = 0x4d42; //"BM"
		pbfh->bfSize = lLenBMP;
		pbfh->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD);

		pbih = (BITMAPINFOHEADER*)(pBMP + sizeof(BITMAPFILEHEADER));
		memset(pbih, 0, sizeof(BITMAPINFOHEADER));
		pbih->biSize = sizeof(BITMAPINFOHEADER);
		pbih->biWidth = iWidth;
		pbih->biHeight = iHeight;
		pbih->biPlanes = 1;
		pbih->biBitCount = iDepth;
		pbih->biXPelsPerMeter = (int)((float)iDPI * 10000 / 254 + .5);
		pbih->biYPelsPerMeter = pbih->biXPelsPerMeter;

		pBMPPtr = pBMP + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER); // point to start of palette
		for (i=0; i<256; i++) {
			pRGBQ = (RGBQUAD*)pBMPPtr;
			pRGBQ->rgbBlue = pRGBQ->rgbGreen = pRGBQ->rgbRed = (char)i;
			pRGBQ->rgbReserved = 0;
			pBMPPtr += sizeof(RGBQUAD);
		}
		// Since the BMP is upside down, move the pointer to the last row
		pBMPPtr += lBytesPerRowBMP*(iHeight-1);

		pRawPtr = pOut;

		// We transfer each row into the new array, with 4-byte alignment
		// and with the rows reversed
		for (i=0; i<iHeight; i++) {
			memset(pBMPPtr, 0, lBytesPerRowBMP);
			memcpy(pBMPPtr, pRawPtr, iWidth);
			pRawPtr += iWidth;			// move down one raw row
			pBMPPtr -= lBytesPerRowBMP;	// move up one BMP row
		}

		free(pOut);  // allocated in wsq_decode_mem via malloc

		GlobalUnlock(*phBMP);

		iRet = 1;
	}

done:

	return iRet;
}

int NISTRAWtoWSQ(char *pRAW, long lWidth, long lHeight, long lDPI, HGLOBAL *phWSQ, float fRate)
{
	int					iRet = 0;
	int					iRetWSQ;
	BYTE				*pOut;
	int					iLenOut;
	void				*pWSQ;

	if (phWSQ == NULL) {
		goto done;
	} else {
		*phWSQ = NULL;
	}

	iRetWSQ = wsq_encode_mem(&pOut, &iLenOut, fRate, pRAW, lWidth, lHeight, 8, lDPI, WSQCOMMENT);

	if (iRetWSQ == 0) {
		
		*phWSQ = GlobalAlloc(GPTR, iLenOut);
		pWSQ = GlobalLock(*phWSQ);
		memcpy(pWSQ, pOut, iLenOut);
		GlobalUnlock(*phWSQ);
		free(pOut); // allocated in wsq_encode_mem via malloc

		iRet = 1;
	}

done:

	return iRet;
}

int NISTWSQtoRAW(char *pWSQ, long lLenWSQ, HGLOBAL *phRAW, long *plWidth, long *plHeight, long *plDPI)
{
	int					iRet = 0;
	int					iRetWSQ;
	BYTE				*pOut;
	int					iWidth;
	int					iHeight;
	int					iDepth;
	int					iDPI;
	int					iLossy;
	BYTE				*pRAW;

	if (phRAW == NULL) {
		goto done;
	} else {
		*phRAW = NULL;
	}

	iRetWSQ = wsq_decode_mem(&pOut, &iWidth, &iHeight, &iDepth, &iDPI, &iLossy, pWSQ, lLenWSQ);

	if (iRetWSQ == 0) {

		// WSQ must be 8 bpp
		if (iDepth != 8) {
			free(pOut);  // allocated in wsq_decode_mem via malloc
			goto done;
		}

		*phRAW = GlobalAlloc(GPTR, iWidth*iHeight);
		if (*phRAW == NULL) goto done;

		pRAW = (BYTE*)GlobalLock(*phRAW);
		if (pRAW == NULL) goto done;

		memcpy(pRAW, pOut, iWidth*iHeight);
		free(pOut);  // allocated in wsq_decode_mem via malloc

		*plWidth = iWidth;
		*plHeight = iHeight;
		*plDPI = iDPI;

		GlobalUnlock(*phRAW);

		iRet = 1;
	}

done:

	return iRet;
}
