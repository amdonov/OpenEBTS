// FreeImageHelpers.h

#ifndef FREEIMAGEHELPERS_H_
#define FREEIMAGEHELPERS_H_

#define FREEIMAGEERRSIZE 1024

FREE_IMAGE_FORMAT FreeImage_GetImageFormat(BYTE* pImage, int cbSize);
FIBITMAP* FreeImage_LoadImageFromMem(BYTE* pImage, int cbSize, FREE_IMAGE_FORMAT fif);
bool FreeImage_SaveImageToMem(FIBITMAP* fibmp, FREE_IMAGE_FORMAT fif, int flags, BYTE** pImageOut, int* pcbSize);
bool FreeImage_ConvertInMemory(FREE_IMAGE_FORMAT fifIn, BYTE* pImageIn, int cbSizeIn, FREE_IMAGE_FORMAT fifOut, int flags, BYTE** pImageOut, int* pcbSizeOut);

void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message);
bool FreeImageError(char szErr[FREEIMAGEERRSIZE]);
void SetFreeImageError(const char szErr[FREEIMAGEERRSIZE]);

#endif /* FREEIMAGEHELPERS_H_ */
