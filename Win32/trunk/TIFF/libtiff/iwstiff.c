// iwstiff.c
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <tchar.h>
#include <windows.h>
#include "tif_config.h"
#include "tiffio.h"
#include "iwstiff.h"

// Routines to override file-based handling
int mfs_open (void *ptr, int size, char *);
int mfs_lseek (thandle_t, toff_t, int whence);
int mfs_read (thandle_t, tdata_t, tsize_t);
int mfs_write (thandle_t,tdata_t, tsize_t);
int mfs_size (thandle_t);
int mfs_close (thandle_t);

int LIBTIFFRAWtoGroup4Fax(char *pRAW, long lWidth, long lHeight, long lDPI, HGLOBAL *phFX4)
// Note: works only for 1bpp RAW input
{
	int		ret = 0;
    HANDLE	fd;
    TIFF	*out = NULL;
    uint32	row;
    uint32	rowsperstrip;
    uint32	bytesperrow;
    char	*pRow = NULL;
    long	lLenFX4;
    uint8   *pFX4 = NULL;
    uint32	i;

	if (pRAW == NULL || phFX4 == NULL ) goto done;

	fd = (HANDLE)mfs_open(NULL, 0, "w");
	out = TIFFClientOpen("dummy", "w", fd, mfs_read, mfs_write, mfs_lseek, mfs_close, mfs_size, NULL, NULL);
	if (out == NULL) goto done;

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, lWidth);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, lHeight);
	TIFFSetField(out, TIFFTAG_XRESOLUTION, lDPI);
	TIFFSetField(out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 1);
	TIFFSetField(out, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_CCITTFAX4);
	TIFFSetField(out, TIFFTAG_FAXMODE, FAXMODE_CLASSF);
	rowsperstrip = TIFFDefaultStripSize(out, -1);
	if (rowsperstrip > (uint32)lHeight) rowsperstrip = lHeight;
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	TIFFSetField(out, TIFFTAG_PAGENUMBER, 0, 1);

	// Compress one row at a time into "out".
	// For some reason PHOTOMETRIC_MINISBLACK gets ignored so the output is negative.
	// We remedy this the easy way by flipping all the bits in the row
	bytesperrow = (lWidth + 7)/8;
	pRow = malloc(bytesperrow);
    for (row = 0; row < (uint32)lHeight; row++)
    {
		for (i = 0; i < bytesperrow; i++)
		{
			*(pRow+i) = ~(*(pRAW+i));
		}
		if (TIFFWriteScanline(out, pRow, row, 0) < 0) goto done;
		pRAW += bytesperrow;
    }
	TIFFWriteDirectory(out);

	// Now "read" it into out output buffer
	lLenFX4 = mfs_size(fd);
	*phFX4 = GlobalAlloc(GMEM_MOVEABLE, lLenFX4);
	if (*phFX4 == NULL) goto done;
	pFX4 = GlobalLock(*phFX4);
	if (pFX4 == NULL) {
		GlobalFree(*phFX4);
		*phFX4 = NULL;
		pFX4 = NULL;
		goto done;
	}

	mfs_lseek(fd, 0, SEEK_SET);
	if (mfs_read(fd, pFX4, lLenFX4) != lLenFX4)
	{
		GlobalUnlock(*phFX4);
		GlobalFree(*phFX4);
		*phFX4 = NULL;
		pFX4 = NULL;
		goto done;
	}

	ret = 1;	// success

done:
	if (pFX4 != NULL) GlobalUnlock(*phFX4);
	if (out != NULL) TIFFClose(out);
	if (pRow != NULL) free(pRow);

	return ret;
}

int LIBTIFFGroup4FaxtoRAW(char *pFX4, long lLenFX4, HGLOBAL *phRAW, long *plWidth, long *plHeight, long *plDPI)
{
	int		ret = 0;
    HANDLE	fd;
    TIFF	*in = NULL;
    uint8	*buf = NULL;
    uint32	row;
    uint32	bytesperrow;
    uint8   *pRAW = NULL;

	if (pFX4 == NULL || phRAW == NULL ) goto done;

	if (!LIBTIFFGetInfo(pFX4, lLenFX4, plWidth, plHeight, plDPI)) goto done;

	fd = (HANDLE)mfs_open(pFX4, lLenFX4, "r");
	in = TIFFClientOpen("dummy", "r", fd, mfs_read, mfs_write, mfs_lseek, mfs_close, mfs_size, NULL, NULL);
	if (in == NULL) goto done;

	// allocate space for one raw decompressed output row
	bytesperrow = (*plWidth + 7)/8;
	buf = _TIFFmalloc(bytesperrow);
    if (buf == 0) goto done;

	*phRAW = GlobalAlloc(GMEM_MOVEABLE, bytesperrow * *plHeight);
	if (*phRAW == NULL) goto done;
	pRAW = GlobalLock(*phRAW);
	if (pRAW == NULL) goto done;

	// decompress one row at a time and copy into return buffer
    for (row = 0; row < (uint32)*plHeight; row++)
    {
		if (TIFFReadScanline(in, buf, row, 0) < 0) goto done;
		memcpy(pRAW, buf, bytesperrow);
		pRAW += bytesperrow;
    }

	ret = 1;	// success

done:
	if (pRAW != NULL) GlobalUnlock(*phRAW);
	if (in != NULL) TIFFClose(in);
	if (buf != NULL) _TIFFfree(buf);

	return ret;
}

int LIBTIFFGetInfo(char *pTIF, long lLenTIF, long *plWidth, long *plHeight, long *plDPI)
// just get some info from the header
{
	int		ret = 0;
    HANDLE	fd;
    TIFF	*in = NULL;
    uint32	resolution;
	uint32  resunit;

	fd = (HANDLE)mfs_open (pTIF, lLenTIF, "r");
	in = TIFFClientOpen("dummy", "r", fd, mfs_read, mfs_write, mfs_lseek, mfs_close, mfs_size, NULL, NULL);
	if (in == NULL) goto done;

	if (TIFFGetField(in, TIFFTAG_IMAGEWIDTH, plWidth) < 0) goto done;
	if (TIFFGetField(in, TIFFTAG_IMAGELENGTH, plHeight) < 0) goto done;
	if (TIFFGetField(in, TIFFTAG_XRESOLUTION, &resolution) < 0) goto done;
	if (TIFFGetField(in, TIFFTAG_RESOLUTIONUNIT, &resunit) < 0) goto done;

	// Set DPI (assume horizontal and vertical resolution is the same)
	if (resunit == RESUNIT_INCH)
	{
		*plDPI = resolution;
	}
	else if (resunit == RESUNIT_CENTIMETER)
	{
		*plDPI = (long)((float)resolution * 2.54 + 0.5);
	}
	else // resunit == RESUNIT_NONE or some undefined value
	{
		*plDPI = 300; // undefined, so let's just choose a number
	}

	ret = 1;	// success

done:
	if (in != NULL) TIFFClose(in);

	return ret;
}
