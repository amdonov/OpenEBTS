

#include "iwsjpeg.h"
#if 1
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#endif // 0

#include "jpeglib.h"

#include "cdjpeg.h"

/*
static const char * const cdjpeg_message_table[] = {
#include "e:\\projects\\jpglib\\cderror.h"
  NULL
};
*/
LOCAL(cjpeg_source_ptr)
 select_file_type(j_compress_ptr cinfo, FILE * infile);

LOCAL(HGLOBAL)
 ConvertDIB(HGLOBAL hSourceDIB, BOOL bBottomUp);

LOCAL(HGLOBAL)
 ConvertDIB_32to24(HGLOBAL hSourceDIB, BOOL bBottomUp);

LOCAL(HGLOBAL)
 ConvertDIB_16to24(HGLOBAL hSourceDIB, BOOL bBottomUp);

LOCAL(int)
 IsValidDIB(HGLOBAL hSourceDIB);

LOCAL(void)
 SaveDIBtoFile(HGLOBAL hSourceDIB, char *path);

LOCAL(int)
 ConvertJPGtoBMP(char *pJPGSrcPath, char *pBMPDestPath, HGLOBAL hSrcJPG, HGLOBAL *phDestBMP);

int BMP_to_JPG(char *pSourcePath, char *pDestPath, int nCompression)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cjpeg_source_ptr src_mgr;
  FILE * input_file;
  FILE * output_file;
  JDIMENSION num_scanlines;
	int nRet = 0;

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  /* Add some application-specific error messages (from cderror.h) 
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;
	*/
  /* Initialize JPEG parameters.
   * Much of this may be overridden later.
   * In particular, we don't yet know the input file's color space,
   * but we need to provide some value for jpeg_set_defaults() to work.
   */

  cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
  jpeg_set_defaults(&cinfo);

	if (nCompression > 25 && nCompression <= 100)
		jpeg_set_quality(&cinfo, nCompression, TRUE); /* requested compression */

  /* Open the input file. */
  if ((input_file = fopen(pSourcePath, READ_BINARY)) == NULL) 
	{
    ERREXITS(&cinfo, IWS_OPEN_FAILED, pSourcePath);
		return 0;
  }

  /* Open the output file. */
  if ((output_file = fopen(pDestPath, WRITE_BINARY)) == NULL) 
	{
		fclose(input_file);
    ERREXITS(&cinfo, IWS_OPEN_FAILED, pDestPath);
		return 0;
	}

  /* Figure out the input file format, and set up to read it. */
  src_mgr = select_file_type(&cinfo, input_file);

	if (!src_mgr) {
		fclose(output_file);
		fclose(input_file);
    ERREXIT(&cinfo, IWS_INVALID_BMP);
		return 0;
	}

  src_mgr->input_file = input_file;
  src_mgr->hDIB = 0;

  /* Read the input file header to obtain file size & colorspace. */
  (*src_mgr->start_input) (&cinfo, src_mgr);

  /* Now that we know input colorspace, fix colorspace-dependent defaults */
  jpeg_default_colorspace(&cinfo);

  /* Adjust default compression parameters by re-parsing the options */
// kas  file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);

  /* Specify data destination for compression */
  jpeg_stdio_dest(&cinfo, output_file);

  /* Start compressor */
  jpeg_start_compress(&cinfo, TRUE);

  /* Process data */
  while (cinfo.next_scanline < cinfo.image_height) {
    num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
    (void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
  }

  /* Finish compression and release memory */
  (*src_mgr->finish_input) (&cinfo, src_mgr);
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  /* Close files, if we opened them */
  if (input_file != stdin)
    fclose(input_file);
  if (output_file != stdout)
    fclose(output_file);

	return nRet;
}

cjpeg_source_ptr select_file_type(j_compress_ptr cinfo, FILE * infile)
{
  int c;

  if ((c = getc(infile)) == EOF)
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
  if (ungetc(c, infile) == EOF)
    ERREXIT(cinfo, JERR_UNGETC_FAILED);

  switch (c) {
  case 'B':
    return jinit_read_bmp(cinfo);
  case 'P':
    return jinit_read_ppm(cinfo);
  default:
    ERREXIT(cinfo, JERR_UNKNOWN_FORMAT);
    break;
  }

  return NULL;			/* suppress compiler warnings */
}

int DIB_to_JPG(HGLOBAL hSourceDIB, char *pDestPath, int nCompression, BOOL bBottomUp)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cjpeg_source_ptr src_mgr;
  FILE * output_file;
  JDIMENSION num_scanlines;
	int nRet = 0;
	HGLOBAL hConvertedDIB = 0;

	if (!hSourceDIB)
		return 0;

	/* the jpg code can only handle 24 and 32 bit dibs */
	if (!IsValidDIB(hSourceDIB))
		return 0;

	/* the jpg code can only handle a max 24 bit dib, we may
	 * have to convert 32 bit dibs to 24 bit dibs in this case
	 */
	hConvertedDIB = ConvertDIB(hSourceDIB, bBottomUp);

	if (hConvertedDIB)
		hSourceDIB = hConvertedDIB;

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  /* Add some application-specific error messages (from cderror.h) 
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;
	*/
  /* Initialize JPEG parameters.
   * Much of this may be overridden later.
   * In particular, we don't yet know the input file's color space,
   * but we need to provide some value for jpeg_set_defaults() to work.
   */

  cinfo.in_color_space = JCS_RGB; /* arbitrary guess */
  jpeg_set_defaults(&cinfo);

	if (nCompression > 25 && nCompression <= 100)
		jpeg_set_quality(&cinfo, nCompression, TRUE); /* requested compression */

  /* Figure out the input file format, and set up to read it. */
  
  src_mgr = (cjpeg_source_ptr)jinit_read_dib(&cinfo, hSourceDIB);

	if (!src_mgr) {
    ERREXIT(&cinfo, IWS_INVALID_DIB);
		return 0;
	}

  /* Open the output file. */
  if ((output_file = fopen(pDestPath, WRITE_BINARY)) == NULL) {
    ERREXITS(&cinfo, IWS_OPEN_FAILED, pDestPath);
		return 0;
	}

  src_mgr->input_file = 0;
  src_mgr->hDIB = hSourceDIB;

  /* Read the input file header to obtain file size & colorspace. */
  if ((*src_mgr->start_input) (&cinfo, src_mgr)) {
		/* Now that we know input colorspace, fix colorspace-dependent defaults */
		jpeg_default_colorspace(&cinfo);

		/* Adjust default compression parameters by re-parsing the options */
	// kas  file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);

		/* Specify data destination for compression */
		jpeg_stdio_dest(&cinfo, output_file);

		/* Start compressor */
		jpeg_start_compress(&cinfo, TRUE);

		/* Process data */
		while (cinfo.next_scanline < cinfo.image_height) {
			num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
			(void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
		}

		/* Finish compression and release memory */
		(*src_mgr->finish_input) (&cinfo, src_mgr);
		jpeg_finish_compress(&cinfo);
	}
	jpeg_destroy_compress(&cinfo);

	nRet = 1;

  /* Close files, if we opened them */
  fclose(output_file);

	if (hConvertedDIB)
		GlobalFree(hConvertedDIB);

	return nRet;
}

HGLOBAL ConvertDIBtoJPG(HGLOBAL hSourceDIB, int nCompression, BOOL bBottomUp)
{
	HGLOBAL hRet = 0;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cjpeg_source_ptr src_mgr;
  JDIMENSION num_scanlines;
	int nRet = 0;
	char *pImage = 0;
	int nSize = 0;
	int errcondition = 0;
	HGLOBAL hConvertedDIB = 0;

	if (!hSourceDIB)
		return 0;

	/* the jpg code can only handle 24 and 32 bit dibs */
	if (!IsValidDIB(hSourceDIB))
		return 0;

	/* the jpg code can only handle a max 24 bit dib, we may
	 * have to convert 32 bit dibs to 24 bit dibs in this case
	 */
	hConvertedDIB = ConvertDIB(hSourceDIB,bBottomUp);

	if (hConvertedDIB)
		hSourceDIB = hConvertedDIB;

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);

  /* Add some application-specific error messages (from cderror.h) 
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;
	*/
  /* Initialize JPEG parameters.
   * Much of this may be overridden later.
   * In particular, we don't yet know the input file's color space,
   * but we need to provide some value for jpeg_set_defaults() to work.
   */

  cinfo.in_color_space = JCS_GRAYSCALE; /* arbitrary guess */
  jpeg_set_defaults(&cinfo);

	if (nCompression > 25 && nCompression <= 100)
		jpeg_set_quality(&cinfo, nCompression, TRUE); /* requested compression */

	/* Figure out the input file format, and set up to read it. */
  
  src_mgr = (cjpeg_source_ptr)jinit_read_dib(&cinfo, hSourceDIB);

	if (!src_mgr)
		goto err;

  src_mgr->input_file = 0;
  src_mgr->hDIB = hSourceDIB;

  /* Read the input file header to obtain file size & colorspace. */
  if ((*src_mgr->start_input) (&cinfo, src_mgr)) {
	  /* Now that we know input colorspace, fix colorspace-dependent defaults */
		jpeg_default_colorspace(&cinfo);

	  /* Specify data destination for compression */
		/*  jpeg_stdio_dest(&cinfo, output_file); */
	
	  jpeg_memory_dest(&cinfo);

		/* Start compressor */
		jpeg_start_compress(&cinfo, TRUE);

		/* Process data */
		while (cinfo.next_scanline < cinfo.image_height) {
			num_scanlines = (*src_mgr->get_pixel_rows) (&cinfo, src_mgr);
			(void) jpeg_write_scanlines(&cinfo, src_mgr->buffer, num_scanlines);
		}

		/* Finish compression and release memory */
		(*src_mgr->finish_input) (&cinfo, src_mgr);
		jpeg_finish_compress(&cinfo);
		pImage = jpeg_get_image(&cinfo, &nSize);
	}

	if (pImage)	{
		char *pNewImage = 0;

		/* create the handle to the jpg and release buffers */
		hRet = GlobalAlloc(GPTR,nSize);

		if (hRet) {
			pNewImage = GlobalLock(hRet);

			if (pNewImage) {
				memcpy(pNewImage,pImage,nSize);
				GlobalUnlock(hRet);
			}
		}
		else {
	    ERREXIT(&cinfo, JERR_OUT_OF_MEMORY);
		}
	}

err: errcondition = 0;

	if (hConvertedDIB)
		GlobalFree(hConvertedDIB);
		
	jpeg_free_image(&cinfo);
  jpeg_destroy_compress(&cinfo);

	return hRet;
}

BOOL AddComment(char *pJPGPath, char *pComment)
{
	return jpeg_add_comment(pJPGPath, pComment);
}

BOOL GetComment(char *pJPGPath, char *pBuffer, int nBufferLen,
									char *pjpginfo, int ninfolen)
{
	return jpeg_get_comment(pJPGPath, pBuffer, nBufferLen, pjpginfo, ninfolen);
}

BOOL ReplaceComment(char *pJPGPath, char *pComment)
{
	return jpeg_replace_comment(pJPGPath, pComment);
}

/* Create the add-on message string table. */

#define JMESSAGE(code,string)	string ,

static const char * const cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};

int JPG_to_BMP(char *pJPGSrcPath, char *pBMPDestPath)
{
	return ConvertJPGtoBMP(pJPGSrcPath, pBMPDestPath, 0,0);
}

int JPGMemory_to_BMP(HGLOBAL hJPG, char *pBMPDestPath)
{
	return ConvertJPGtoBMP(0, pBMPDestPath, hJPG,0);
}

int JPGMemory_to_BMPMemory(HGLOBAL hJPG, HGLOBAL *phBMP)
{
	return ConvertJPGtoBMP(0,0,hJPG,phBMP);
}

int BMPMemory_to_JPGMemory(HGLOBAL hBMP, HGLOBAL *phJPG, int nCompression)
// There are no in-memory BMP functions so we have to take out the bitmap
// file header before passing it to the JPEG lib.
{
	int		iRet = 0;
	HGLOBAL hDIB = NULL;
	BYTE	*pBMP = NULL;
	BYTE	*pDIB = NULL;
	long	 lLenBMP;
	long	 lLenDIB;

	pBMP = GlobalLock(hBMP);
	if (pBMP == NULL) goto done;
	lLenBMP = GlobalSize(hBMP);
	lLenDIB = lLenBMP - sizeof(BITMAPFILEHEADER);

	hDIB = GlobalAlloc(GMEM_MOVEABLE, lLenDIB);
	pDIB = GlobalLock(hDIB);
	if (pDIB == NULL) goto done;

	memcpy(pDIB, pBMP + sizeof(BITMAPFILEHEADER), lLenDIB);
	GlobalUnlock(hDIB);
	pDIB = NULL;

	*phJPG = ConvertDIBtoJPG(hDIB, nCompression, FALSE);

	iRet = 1; // success

done:
	if (pBMP != NULL) GlobalUnlock(hBMP);
	if (pDIB != NULL) GlobalUnlock(hDIB);
	if (hDIB != NULL) GlobalFree(hDIB);

	return iRet;
}

int JPG_GetInfo(HGLOBAL hJPG, IWJPGINFO *jpgInfo)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  djpeg_dest_ptr dest_mgr = NULL;
  FILE * input_file = 0;
  FILE * output_file = 0;
//  JDIMENSION num_scanlines;
	unsigned char * input_file_memory = 0;
	long input_file_memory_size = 0;
	HGLOBAL hSrcJPG = hJPG;

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	/* Add some application-specific error messages (from cderror.h) 
	jerr.addon_message_table = cdjpeg_message_table;
	jerr.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr.last_addon_message = JMSG_LASTADDONCODE;
	*/

	if (hSrcJPG) {
		input_file_memory_size = GlobalSize(hSrcJPG);
		input_file_memory = (unsigned char *)GlobalLock(hSrcJPG);
	}
	else {
		ERREXIT(&cinfo, IWS_MISSING_JPG);
		return IWS_MISSING_JPG;
	}

	/* Specify data source for decompression */
	jpeg_stdio_src(&cinfo, input_file, input_file_memory, input_file_memory_size);

	/* Read file header, set default decompression parameters */
	(void) jpeg_read_header(&cinfo, TRUE);
	
	jpgInfo->lWidth = cinfo.image_width;
	jpgInfo->lHeight = cinfo.image_height;
	jpgInfo->lScaleUnits = cinfo.density_unit; // 1 == pixels/inch, 2 == pixels/centimeter
	jpgInfo->lHPS = cinfo.X_density; // horizontal pixel scale
	jpgInfo->lVPS = cinfo.Y_density; // vertical pixel scale
	jpgInfo->lCompression = 1; // 1 indicates baseline jpg(JPEGB), 2 indicates lossless compression (JPEGL)
	jpgInfo->lColorSpace = (long)cinfo.jpeg_color_space; // one of the above defines, CS_YCbCr == NIST YCC

//  (void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 1;
}

LOCAL(int)
 ConvertJPGtoBMP(char *pJPGSrcPath, char *pBMPDestPath, HGLOBAL hSrcJPG, HGLOBAL *phDestBMP)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  djpeg_dest_ptr dest_mgr = NULL;
  FILE * input_file = 0;
  FILE * output_file = 0;
  JDIMENSION num_scanlines;
	unsigned char * input_file_memory = 0;
	long input_file_memory_size = 0;
	
  /* Initialize the JPEG decompression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  /* Add some application-specific error messages (from cderror.h) 
  jerr.addon_message_table = cdjpeg_message_table;
  jerr.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr.last_addon_message = JMSG_LASTADDONCODE;
	*/

	if (pJPGSrcPath) {
		/* Open the input file. */
		if ((input_file = fopen(pJPGSrcPath, READ_BINARY)) == NULL) {
			ERREXITS(&cinfo, IWS_OPEN_FAILED, pJPGSrcPath);
			return IWS_OPEN_FAILED;
		}
	}
	else if (hSrcJPG) {
		input_file_memory_size = GlobalSize(hSrcJPG);
		input_file_memory = (unsigned char *)GlobalLock(hSrcJPG);
	}
	else {
    ERREXIT(&cinfo, IWS_MISSING_JPG);
    return IWS_MISSING_JPG;
	}

	if (pBMPDestPath) {
		/* Open the output file. */
		if ((output_file = fopen(pBMPDestPath, WRITE_BINARY)) == NULL) {
			ERREXITS(&cinfo, IWS_OPEN_FAILED, pBMPDestPath);
			return IWS_OPEN_FAILED;
		 }
	}

  /* Specify data source for decompression */
  jpeg_stdio_src(&cinfo, input_file, input_file_memory, input_file_memory_size);

  /* Read file header, set default decompression parameters */
  (void) jpeg_read_header(&cinfo, TRUE);

/*DJD - now supports grayscale	
	if (cinfo.out_color_space == JCS_GRAYSCALE) {
		if (input_file)
			fclose(input_file);
		if (output_file)
			fclose(output_file);
			
		ERREXIT(&cinfo, IWS_INVALID_FORMAT);
		return IWS_INVALID_FORMAT;
	}
*/

  dest_mgr = jinit_write_bmp(&cinfo, FALSE, (phDestBMP ? TRUE : FALSE));

  dest_mgr->output_file = output_file;

  /* Start decompressor */
  (void) jpeg_start_decompress(&cinfo);

  /* Write output file header */
  (*dest_mgr->start_output) (&cinfo, dest_mgr);

  /* Process data */
  while (cinfo.output_scanline < cinfo.output_height) {
    num_scanlines = jpeg_read_scanlines(&cinfo, dest_mgr->buffer,
					dest_mgr->buffer_height);
    (*dest_mgr->put_pixel_rows) (&cinfo, dest_mgr, num_scanlines);
  }

  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  (*dest_mgr->finish_output) (&cinfo, dest_mgr);

	/* grab photo before cleanup */
	if (phDestBMP)
	{
		if (0) // bFlipBits)
		{
			LPBITMAPFILEHEADER pBMFInfo = (LPBITMAPFILEHEADER)GlobalLock(dest_mgr->output_file_memory);
			LPBITMAPINFOHEADER pBMInfo = (LPBITMAPINFOHEADER)((unsigned char *)pBMFInfo+sizeof(BITMAPFILEHEADER));
			int i = 0;
			LPBYTE lpData = 0;
			int nRowBytes = 0;
			LPBYTE lpBits = 0;
			LPBYTE lpSrc = 0;
			LPBYTE lpDest = 0;

			pBMInfo->biSizeImage = ((pBMInfo->biBitCount/8)*pBMInfo->biWidth)*pBMInfo->biHeight;
			
			// flip the buffer
			lpData = calloc(pBMInfo->biSizeImage,sizeof(char));
			nRowBytes = (pBMInfo->biBitCount/8)*pBMInfo->biWidth;
			lpBits = (BYTE *)pBMInfo+pBMInfo->biSize;
			lpSrc = lpBits;
			lpDest = lpData+((pBMInfo->biHeight-1)*nRowBytes);

			for (i = 0; i < pBMInfo->biHeight; i++)
			{
				lpSrc = lpBits+(nRowBytes*i);
				memcpy(lpDest,lpSrc,nRowBytes);
				lpDest -= nRowBytes;
			}

			// copy flipped bits back
			memcpy((BYTE *)pBMInfo+pBMInfo->biSize,lpData,pBMInfo->biSizeImage);
			free(lpData);

			GlobalUnlock(dest_mgr->output_file_memory);

			*phDestBMP = dest_mgr->output_file_memory;
		}
		else
			*phDestBMP = dest_mgr->output_file_memory;
	}
	 
  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  /* Close files, if we opened them */
  if (input_file)
    fclose(input_file);
  if (output_file)
    fclose(output_file);

	return 1;
}

void SetLogMode(int nMode, int nMsg_level)
{
	g_bLogMode = nMode;
	g_MessageLevel = nMsg_level;
}

BOOL SetLogFile(char *pDestFile)
{
	FILE *logfile;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);

	g_szLogFilePath[0] = '\0';

  /* attempt to open the log file. */
  if ((logfile = fopen(pDestFile, APPEND_TEXT)) == NULL) {
    ERREXITS(&cinfo, IWS_OPEN_FAILED, pDestFile);
		return FALSE;
	}
	fclose(logfile);

	strcpy(g_szLogFilePath,pDestFile);

	return TRUE;
}

LOCAL(int)
 IsValidDIB(HGLOBAL hSourceDIB)
{
	int nRet = 0;
	HGLOBAL hRet = 0;
	BITMAPINFOHEADER *pBMI;
	
	if (hSourceDIB) {
		pBMI = GlobalLock(hSourceDIB);
		if (pBMI && pBMI->biSize == sizeof(BITMAPINFOHEADER)) {
			if (pBMI->biBitCount == 32 || pBMI->biBitCount == 24 || pBMI->biBitCount == 16 || pBMI->biBitCount == 8) 
				nRet = 1;
		}
		GlobalUnlock(hSourceDIB);
	}

	return nRet;
}

LOCAL(HGLOBAL)
 ConvertDIB(HGLOBAL hSourceDIB, BOOL bBottomUp)
{
	HGLOBAL hRet = 0;
	BITMAPINFOHEADER *pBMI;
	int bitCount;
	BOOL bMask;

//	SaveDIBtoFile(hSourceDIB, "e:\\32bit.bmp");
	
	if (hSourceDIB) {
		pBMI = GlobalLock(hSourceDIB);
		if (pBMI && pBMI->biSize == sizeof(BITMAPINFOHEADER)) {
			bitCount = pBMI->biBitCount;
			bMask = pBMI->biCompression == BI_RGB; // if not BI_RGB then we can't convert 16 bit images
			GlobalUnlock(hSourceDIB);

			if (bitCount == 32)
				hRet = ConvertDIB_32to24(hSourceDIB,bBottomUp);
			else if (bitCount == 16 && bMask) 
				hRet = ConvertDIB_16to24(hSourceDIB,bBottomUp);
		}
	}

	return hRet;
}

LOCAL(HGLOBAL)
 ConvertDIB_32to24(HGLOBAL hSourceDIB, BOOL bBottomUp)
{
	HGLOBAL hRet = 0;
	BITMAPINFOHEADER *pBMI;
	BITMAPINFOHEADER *pBMICopy;
	BYTE *pSrcBits;
	BYTE *pDestBits;

	if (hSourceDIB) {
		pBMI = GlobalLock(hSourceDIB);
		if (pBMI && pBMI->biSize == sizeof(BITMAPINFOHEADER)) {
			if (pBMI->biBitCount == 32) {
				hRet = GlobalAlloc(GPTR,(pBMI->biWidth*pBMI->biHeight*3)+sizeof(BITMAPINFOHEADER));
				if (hRet) {
					pBMICopy = (BITMAPINFOHEADER *)GlobalLock(hRet);
					if (pBMICopy) {
						int nPixels = pBMI->biWidth*pBMI->biHeight;
						int i,j;
						BYTE *pRowBits;

						memcpy(pBMICopy,pBMI,sizeof(BITMAPINFOHEADER));
						pDestBits = ((char *)pBMICopy)+(char)(sizeof(BITMAPINFOHEADER));
						
						/* 
						 * if bBottomUp, then image starts at lower right corner 
						*/

						if (bBottomUp) {
							pSrcBits = ((char *)pBMI)+(char)((sizeof(BITMAPINFOHEADER))+pBMI->biSizeImage);

							/* copy the image bits, skipping the empty high byte in each R,G,B */
							for (i = pBMI->biHeight; i > 0 ; i--) {
								pRowBits = pSrcBits+((i-1)*pBMI->biWidth*4);
								for (j = 0; j < pBMI->biWidth; j++) {
									pDestBits[0] = *pRowBits++;
									pDestBits[1] = *pRowBits++;
									pDestBits[2] = *pRowBits++;
									pRowBits++; /* skip the high byte */
									pDestBits += 3;
								}
							}
						}
						else {
							pSrcBits = ((char *)pBMI)+(char)((sizeof(BITMAPINFOHEADER)));

							/* copy the image bits, skipping the empty high byte in each R,G,B */
							for (i = 0; i < pBMI->biHeight; i++) {
								pRowBits = pSrcBits+((i*pBMI->biWidth*4));
								for (j = 0; j < pBMI->biWidth; j++) {
									pDestBits[0] = *pRowBits++;
									pDestBits[1] = *pRowBits++;
									pDestBits[2] = *pRowBits++;
									pRowBits++; /* skip the high byte */
									pDestBits += 3;
								}
							}
						}
					}
					pBMICopy->biBitCount = 24;
					pBMICopy->biSizeImage = pBMI->biWidth*pBMI->biHeight*3;
					GlobalUnlock(hRet);
				}
			}
			GlobalUnlock(hSourceDIB);
		}
	}

	return hRet;
}

#define BLUE_MASK		0x001F	// binary 000000000011111
#define GREEN_MASK	0x03E0	// binary 000001111100000
#define RED_MASK		0x7C00	// binary 111110000000000

LOCAL(HGLOBAL)
 ConvertDIB_16to24(HGLOBAL hSourceDIB, BOOL bBottomUp)
{
	HGLOBAL hRet = 0;
	BITMAPINFOHEADER *pBMI;
	BITMAPINFOHEADER *pBMICopy;
	BYTE *pSrcBits;
	BYTE *pDestBits;

	/*
	 * 16 bit images values range from 0-31, simply multiply by 8 (<< 3) to
	 * convert to 24 bit values 0-255. 
	 * We only handle 16 bit type 555 (RGB), there is also a 16 bit 565 (RGB)
	 */

	if (hSourceDIB) {
		pBMI = GlobalLock(hSourceDIB);
		if (pBMI && pBMI->biSize == sizeof(BITMAPINFOHEADER)) {
			if (pBMI->biBitCount == 16) {
				hRet = GlobalAlloc(GPTR,(pBMI->biWidth*pBMI->biHeight*3)+sizeof(BITMAPINFOHEADER));
				if (hRet) {
					pBMICopy = (BITMAPINFOHEADER *)GlobalLock(hRet);
					if (pBMICopy) {
						int nPixels = pBMI->biWidth*pBMI->biHeight;
						int i,j;
						WORD *pRowBits;

						memcpy(pBMICopy,pBMI,sizeof(BITMAPINFOHEADER));
						pDestBits = ((char *)pBMICopy)+(char)(sizeof(BITMAPINFOHEADER));
						
						/* 
						 * if bBottomUp, then image starts at lower right corner 
						*/

						if (bBottomUp) {
							pSrcBits = ((char *)pBMI)+(char)((sizeof(BITMAPINFOHEADER))+pBMI->biSizeImage);

							for (i = pBMI->biHeight; i > 0 ; i--) {
								pRowBits = (WORD *)((char *)pSrcBits+((i-1)*pBMI->biWidth*sizeof(WORD)));
								for (j = 0; j < pBMI->biWidth; j++) {
									pDestBits[0] = ((BLUE_MASK & *pRowBits) << 3);
									pDestBits[1] = ((GREEN_MASK & *pRowBits) >> 5) << 3;
									pDestBits[2] = ((RED_MASK & *pRowBits) >> 10) << 3;
									pRowBits++; /* skip the high byte */
									pDestBits += 3;
								}
							}
						}
						else {
							pSrcBits = ((char *)pBMI)+(char)((sizeof(BITMAPINFOHEADER)));

							for (i = 0; i < pBMI->biHeight; i++) {
								pRowBits = (WORD *)((char *)pSrcBits+((i*pBMI->biWidth*sizeof(WORD))));
								for (j = 0; j < pBMI->biWidth; j++) {
									pDestBits[0] = ((BLUE_MASK & *pRowBits) << 3);
									pDestBits[1] = ((GREEN_MASK & *pRowBits) >> 5) << 3;
									pDestBits[2] = ((RED_MASK & *pRowBits) >> 10) << 3;
									pRowBits++; /* skip the high byte */
									pDestBits += 3;
								}
							}
						}
					}
					pBMICopy->biBitCount = 24;
					pBMICopy->biSizeImage = pBMI->biWidth*pBMI->biHeight*3;
					GlobalUnlock(hRet);
				}
			}
			GlobalUnlock(hSourceDIB);
		}
	}

	return hRet;
}

LOCAL(void)
 SaveDIBtoFile(HGLOBAL hSourceDIB, char *path)
{
	if (hSourceDIB)
	{
		BITMAPINFOHEADER *lpbmInfo = GlobalLock(hSourceDIB);
		FILE *output_file;
		LPBITMAPFILEHEADER lpBMFileHdr = 0;

		long lSize = sizeof(BITMAPINFOHEADER)+lpbmInfo->biSizeImage;

		lpBMFileHdr = (LPBITMAPFILEHEADER) GlobalAlloc(GPTR,lSize+sizeof(BITMAPFILEHEADER));

		memcpy((LPBYTE)lpBMFileHdr+sizeof(BITMAPFILEHEADER), lpbmInfo, lSize);

		lpBMFileHdr->bfType = 0x4d42; // BM
		lpBMFileHdr->bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
						 lpbmInfo->biSize + lpbmInfo->biClrUsed 
						 * sizeof(RGBQUAD) + lpbmInfo->biSizeImage); 

		lpBMFileHdr->bfReserved1 = 0; 
		lpBMFileHdr->bfReserved2 = 0; 

		lpBMFileHdr->bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
								lpbmInfo->biSize + lpbmInfo->biClrUsed * sizeof(RGBQUAD); 

		if ((output_file = fopen(path, "wb")) != NULL) 
		{
			fwrite(lpBMFileHdr, sizeof(char), lSize+sizeof(BITMAPFILEHEADER), output_file);
			fclose(output_file);
		}

		GlobalFree((HGLOBAL)lpBMFileHdr);
		GlobalUnlock(hSourceDIB);
	}
}