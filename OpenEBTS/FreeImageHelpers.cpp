// FreeImageHelpers.cpp
//
// Some useful extensions to the FreeImage library.
//

#include "Includes.h"


FREE_IMAGE_FORMAT FreeImage_GetImageFormat(BYTE* pImage, int cbSize)
{
	FREE_IMAGE_FORMAT	fif = FIF_UNKNOWN;
	FIMEMORY			*fiStream = NULL;
	char				szErr[FREEIMAGEERRSIZE] = { 0 };

	// Get FreeImage "memory handle"
	fiStream = FreeImage_OpenMemory(pImage, cbSize);
	if (FreeImageError(szErr)) goto Exit;

	fif = FreeImage_GetFileTypeFromMemory(fiStream);
	if (FreeImageError(szErr)) goto Exit;

Exit:
	// Release FreeImage "memory handle"
	if (fiStream) FreeImage_CloseMemory(fiStream);

	// Reset global error so that caller can get at it by interrogating FreeImageError()
	if (szErr[0] != '\0') SetFreeImageError(szErr);

	return fif;
}

FIBITMAP* FreeImage_LoadImageFromMem(BYTE* pImage, int cbSize, FREE_IMAGE_FORMAT fif)
{
	FIBITMAP			*fibmp = NULL;
	FIMEMORY			*fiStream = NULL;
	char				szErr[FREEIMAGEERRSIZE] = { 0 };

	// Get FreeImage "memory handle"
	fiStream = FreeImage_OpenMemory(pImage, cbSize);
	if (FreeImageError(szErr)) goto Exit;

	fibmp = FreeImage_LoadFromMemory(fif, fiStream);
	if (FreeImageError(szErr)) goto Exit;

Exit:
	// Release FreeImage "memory handle"
	if (fiStream) FreeImage_CloseMemory(fiStream);

	// Reset global error so that caller can get at it by interrogating FreeImageError()
	if (szErr[0] != '\0') SetFreeImageError(szErr);

	return fibmp;
}

bool FreeImage_SaveImageToMem(FIBITMAP* fibmp, FREE_IMAGE_FORMAT fif, int flags, BYTE** pImageOut, int* pcbSize)
{
	bool				bRet = false;
	FIMEMORY			*fiStream = NULL;
	BYTE				*pFreeImageMem = NULL;
	DWORD				dwSize;
	char				szErr[FREEIMAGEERRSIZE] = { 0 };

	if (pImageOut == NULL || pcbSize == NULL)
	{
		SetFreeImageError("Null parameter");
		return false;
	}

	// Get FreeImage "memory handle"
	fiStream = FreeImage_OpenMemory();
	if (FreeImageError(szErr)) goto Exit;

	if (!FreeImage_SaveToMemory(fif, fibmp, fiStream, flags)) goto Exit;
	if (FreeImageError(szErr)) goto Exit;

	// Copy from FreeImage stream to our BYTE*
	FreeImage_AcquireMemory(fiStream, &pFreeImageMem, &dwSize);
	*pImageOut = new BYTE [dwSize];
	memcpy(*pImageOut, pFreeImageMem, dwSize);

	*pcbSize = dwSize;
	bRet = true;

Exit:
	// Release FreeImage "memory handle"
	if (fiStream) FreeImage_CloseMemory(fiStream);

	// Reset global error so that caller can get at it by interrogating FreeImageError()
	if (szErr[0] != '\0') SetFreeImageError(szErr);

	return bRet;
}

bool FreeImage_ConvertInMemory(FREE_IMAGE_FORMAT fifIn, BYTE* pImageIn, int cbSizeIn, FREE_IMAGE_FORMAT fifOut, int flags, BYTE** pImageOut, int* pcbSizeOut)
{
	bool				bRet = false;
	char				szErr[FREEIMAGEERRSIZE] = { 0 };
	FIBITMAP			*fibmp = NULL;

	if (pImageIn == NULL || pImageOut == NULL || pcbSizeOut == NULL)
	{
		SetFreeImageError("Null parameter");
		return false;
	}

	fibmp = FreeImage_LoadImageFromMem(pImageIn, cbSizeIn, fifIn);
	if (FreeImageError(szErr)) goto Exit;

	FreeImage_SaveImageToMem(fibmp, fifOut, flags, pImageOut, pcbSizeOut);
	if (FreeImageError(szErr)) goto Exit;

	bRet = true;

Exit:
	if (fibmp != NULL) FreeImage_Unload(fibmp);

	// Reset global error so that caller can get at it by interrogating FreeImageError()
	if (szErr[0] != '\0') SetFreeImageError(szErr);

	return bRet;
}

//
// Global error handler for FreeImage functions
//

char g_szLastErr[FREEIMAGEERRSIZE];	// Global FreeImage error message container

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
// FreeImage error handler. Caller will check g_szLastErr with
// function CheckFreeImageError(), which will reset the error string
// for the next call.
{
	g_szLastErr[0] = '\0';

	if (fif != FIF_UNKNOWN)
	{
		strcpy(g_szLastErr, "(");
		strcat(g_szLastErr, FreeImage_GetFormatFromFIF(fif));
		strcat(g_szLastErr, ") ");
	}
	strcat(g_szLastErr, message);
}

bool FreeImageError(char szErr[FREEIMAGEERRSIZE])
{
	if (g_szLastErr[0] == '\0')
	{
		return false;
	}
	else
	{
		strcpy(szErr, g_szLastErr);
		g_szLastErr[0] = '\0';	// empty error once it has been 'observed'
		return true;
	}
}

void SetFreeImageError(const char szErr[FREEIMAGEERRSIZE])
{
	strcpy(g_szLastErr, szErr);
}
