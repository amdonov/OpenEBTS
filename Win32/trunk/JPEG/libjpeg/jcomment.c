

#include "iwsjpeg.h"
#include <stdio.h>
#include <io.h>
#include <time.h>
#include "jconfig.h"
#include "jmorecfg.h"
#include "cdjpeg.h"

#ifdef DONT_USE_B_MODE		/* define mode parameters for fopen() */
#define READ_BINARY	"r"
#define RW_BINARY	"w+"
#else
#ifdef VMS			/* VMS is very nonstandard */
#define READ_BINARY	"rb", "ctx=stm"
#define RW_BINARY	"w+b", "ctx=stm"
#else				/* standard ANSI-compliant case */
#define READ_BINARY	"rb"
//#define WRITE_BINARY	"w+b"
#endif
#endif

	/* code taken from wrjpgcom.c and made to fit in a library environment */
/*
 * wrjpgcom.c
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a very simple stand-alone application that inserts
 * user-supplied text as a COM (comment) marker in a JFIF file.
 * This may be useful as an example of the minimum logic needed to parse
 * JPEG markers.
 */

static FILE *infile;		/* input JPEG file */
//static FILE *outfile;		/* modified JPEG file */

typedef struct _JPGMEMFILE_
{
	int nlen;
	BYTE *pjpgfile;
	int nmaxbuffersize;
	int nerrors;
} JPGMEMFILE;

static JPGMEMFILE *pjpgmemfile = 0;

/* Return next input byte, or EOF if no more */
#define NEXTBYTE()  getc(infile)
/* Emit an output byte */
/* #define PUTBYTE(x)  putc((x), outfile) */

/* Error exit handler */
/* #define ERREXIT(msg)  (fprintf(stderr, "%s\n", msg), exit(EXIT_FAILURE)) */

/* Note that mktemp() requires the initial filename to end in six X's */
#ifndef TEMP_FILE_NAME	/* can override from jconfig.h or Makefile */
#define TEMP_FILE_NAME  "%sJPG%03dXXXXXX"
#endif

LOCAL(void)
select_file_name (char * fname)
{
//  next_file_num++;		/* advance counter */
//  sprintf(fname, TEMP_FILE_NAME, TEMP_DIRECTORY, next_file_num);
  _mktemp(fname);		/* make sure file name is unique */
  /* mktemp replaces the trailing XXXXXX with a unique string of characters */
}

/* Read one byte, testing for EOF */
LOCAL(int)
read_1_byte (j_compress_ptr jerr)
{
  int c;

  c = NEXTBYTE();
  if (c == EOF)
    ERREXIT(jerr,JERR_INPUT_EOF);
  return c;
}

/* Read 2 bytes, convert to unsigned int */
/* All 2-byte quantities in JPEG markers are MSB first */
LOCAL(unsigned int)
read_2_bytes (j_compress_ptr jerr)
{
  int c1, c2;

  c1 = NEXTBYTE();
  if (c1 == EOF)
    ERREXIT(jerr,JERR_INPUT_EOF);
  c2 = NEXTBYTE();
  if (c2 == EOF)
    ERREXIT(jerr,JERR_INPUT_EOF);
  return (((unsigned int) c1) << 8) + ((unsigned int) c2);
}

LOCAL(void)
PUTBYTE (int c)
{
	if (pjpgmemfile->nlen+1 <= pjpgmemfile->nmaxbuffersize) {
		pjpgmemfile->pjpgfile[pjpgmemfile->nlen] = c;
		pjpgmemfile->nlen++;
	}
	else
		pjpgmemfile->nerrors++;
}

/* Routines to write data to output file */

LOCAL(void)
write_1_byte (int c)
{
  PUTBYTE(c);
}

LOCAL(void)
write_2_bytes (unsigned int val)
{
  PUTBYTE((val >> 8) & 0xFF);
  PUTBYTE(val & 0xFF);
}

LOCAL(void)
write_marker (int marker)
{
  PUTBYTE(0xFF);
  PUTBYTE(marker);
}

LOCAL(void)
copy_rest_of_file (void)
{
  int c;

  while ((c = NEXTBYTE()) != EOF)
    PUTBYTE(c);
}


/*
 * JPEG markers consist of one or more 0xFF bytes, followed by a marker
 * code byte (which is not an FF).  Here are the marker codes of interest
 * in this program.  (See jdmarker.c for a more complete list.)
 */

#define M_SOF0  0xC0		/* Start Of Frame N */
#define M_SOF1  0xC1		/* N indicates which compression process */
#define M_SOF2  0xC2		/* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5		/* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8		/* Start Of Image (beginning of datastream) */
#define M_EOI   0xD9		/* End Of Image (end of datastream) */
#define M_SOS   0xDA		/* Start Of Scan (begins compressed data) */
#define M_APP0	0xE0		/* Application-specific marker, type N */
#define M_APP12	0xEC		/* (we don't bother to list all 16 APPn's) */
#define M_COM   0xFE		/* COMment */


/*
 * Find the next JPEG marker and return its marker code.
 * We expect at least one FF byte, possibly more if the compressor used FFs
 * to pad the file.  (Padding FFs will NOT be replicated in the output file.)
 * There could also be non-FF garbage between markers.  The treatment of such
 * garbage is unspecified; we choose to skip over it but emit a warning msg.
 * NB: this routine must not be used after seeing SOS marker, since it will
 * not deal correctly with FF/00 sequences in the compressed image data...
 */

LOCAL(int)
next_marker (j_compress_ptr jerr)
{
  int c;
  int discarded_bytes = 0;

  /* Find 0xFF byte; count and skip any non-FFs. */
  c = read_1_byte(jerr);
  while (c != 0xFF) {
    discarded_bytes++;
    c = read_1_byte(jerr);
  }
  /* Get marker code byte, swallowing any duplicate FF bytes.  Extra FFs
   * are legal as pad bytes, so don't count them in discarded_bytes.
   */
  do {
    c = read_1_byte(jerr);
  } while (c == 0xFF);

  if (discarded_bytes != 0) {
    fprintf(stderr, "Warning: garbage data found in JPEG file\n");
  }

  return c;
}


/*
 * Read the initial marker, which should be SOI.
 * For a JFIF file, the first two bytes of the file should be literally
 * 0xFF M_SOI.  To be more general, we could use next_marker, but if the
 * input file weren't actually JPEG at all, next_marker might read the whole
 * file and then return a misleading error message...
 */

LOCAL(int)
first_marker (j_compress_ptr jerr)
{
  int c1, c2;

  c1 = NEXTBYTE();
  c2 = NEXTBYTE();
  if (c1 != 0xFF || c2 != M_SOI)
    ERREXIT(jerr,IWS_NOT_JPEG);
  return c2;
}

/*
 * Most types of marker are followed by a variable-length parameter segment.
 * This routine skips over the parameters for any marker we don't otherwise
 * want to process.
 * Note that we MUST skip the parameter segment explicitly in order not to
 * be fooled by 0xFF bytes that might appear within the parameter segment;
 * such bytes do NOT introduce new markers.
 */

LOCAL(void)
copy_variable (j_compress_ptr jerr)
/* Copy an unknown or uninteresting variable-length marker */
{
  unsigned int length;

  /* Get the marker parameter length count */
  length = read_2_bytes(jerr);
  write_2_bytes(length);
  /* Length includes itself, so must be at least 2 */
  if (length < 2)
    ERREXIT(jerr,JERR_BAD_LENGTH);
  length -= 2;
  /* Skip over the remaining bytes */
  while (length > 0) {
    write_1_byte(read_1_byte(jerr));
    length--;
  }
}

LOCAL(void)
skip_variable (j_compress_ptr jerr)
/* Skip over an unknown or uninteresting variable-length marker */
{
  unsigned int length;

  /* Get the marker parameter length count */
  length = read_2_bytes(jerr);
  /* Length includes itself, so must be at least 2 */
  if (length < 2)
    ERREXIT(jerr,JERR_BAD_LENGTH);
  length -= 2;
  /* Skip over the remaining bytes */
  while (length > 0) {
    (void) read_1_byte(jerr);
    length--;
  }
}


/*
 * Parse the marker stream until SOFn or EOI is seen;
 * copy data to output, but discard COM markers unless keep_COM is true.
 */

LOCAL(int)
scan_JPEG_header (j_compress_ptr jerr, int keep_COM)
{
  int marker;

  /* Expect SOI at start of file */
  if (first_marker(jerr) != M_SOI)
    ERREXIT(jerr,IWS_NO_SOI);
  write_marker(M_SOI);

  /* Scan miscellaneous markers until we reach SOFn. */
  for (;;) {
    marker = next_marker(jerr);
    switch (marker) {
      /* Note that marker codes 0xC4, 0xC8, 0xCC are not, and must not be,
       * treated as SOFn.  C4 in particular is actually DHT.
       */
    case M_SOF0:		/* Baseline */
    case M_SOF1:		/* Extended sequential, Huffman */
    case M_SOF2:		/* Progressive, Huffman */
    case M_SOF3:		/* Lossless, Huffman */
    case M_SOF5:		/* Differential sequential, Huffman */
    case M_SOF6:		/* Differential progressive, Huffman */
    case M_SOF7:		/* Differential lossless, Huffman */
    case M_SOF9:		/* Extended sequential, arithmetic */
    case M_SOF10:		/* Progressive, arithmetic */
    case M_SOF11:		/* Lossless, arithmetic */
    case M_SOF13:		/* Differential sequential, arithmetic */
    case M_SOF14:		/* Differential progressive, arithmetic */
    case M_SOF15:		/* Differential lossless, arithmetic */
      return marker;

    case M_SOS:			/* should not see compressed data before SOF */
      ERREXIT(jerr,IWS_SOS_NO_SOF);
      break;

    case M_EOI:			/* in case it's a tables-only JPEG stream */
      return marker;

    case M_COM:			/* Existing COM: conditionally discard */
			if (keep_COM) {
	write_marker(marker);
	copy_variable(jerr);
      } else {
	skip_variable(jerr);
      }
      break;

    default:			/* Anything else just gets copied */
      write_marker(marker);
      copy_variable(jerr);		/* we assume it has a parameter count... */
      break;
    }
  } /* end loop */
}

/*
 * Process a SOFn marker.
 * This code is only needed if you want to know the image dimensions...
 */

LOCAL(void)
process_SOFn (j_compress_ptr jerr, char *pinfo, int nmaxinfolen, int marker)
{
  unsigned int length;
  unsigned int image_height, image_width;
  int data_precision, num_components;
  const char * process;
  int ci;
	char info[255] = { '\0', };

  length = read_2_bytes(jerr);	/* usual parameter length count */

  data_precision = read_1_byte(jerr);
  image_height = read_2_bytes(jerr);
  image_width = read_2_bytes(jerr);
  num_components = read_1_byte(jerr);

  switch (marker) {
  case M_SOF0:	process = "Baseline";  break;
  case M_SOF1:	process = "Extended sequential";  break;
  case M_SOF2:	process = "Progressive";  break;
  case M_SOF3:	process = "Lossless";  break;
  case M_SOF5:	process = "Differential sequential";  break;
  case M_SOF6:	process = "Differential progressive";  break;
  case M_SOF7:	process = "Differential lossless";  break;
  case M_SOF9:	process = "Extended sequential, arithmetic coding";  break;
  case M_SOF10:	process = "Progressive, arithmetic coding";  break;
  case M_SOF11:	process = "Lossless, arithmetic coding";  break;
  case M_SOF13:	process = "Differential sequential, arithmetic coding";  break;
  case M_SOF14:	process = "Differential progressive, arithmetic coding"; break;
  case M_SOF15:	process = "Differential lossless, arithmetic coding";  break;
  default:	process = "Unknown";  break;
  }

  sprintf(info, "[JPEG] info: process %s, image is %uw * %uh, %d color components, %d bits per sample",
	 process, image_width, image_height, num_components, data_precision);

	if ((int)strlen(pinfo)+(int)strlen(info) < nmaxinfolen)
		strcat(pinfo,info);

  if (length != (unsigned int) (8 + num_components * 3))
    ERREXIT(jerr,JERR_BAD_LENGTH);

  for (ci = 0; ci < num_components; ci++) {
    (void) read_1_byte(jerr);	/* Component ID code */
    (void) read_1_byte(jerr);	/* H, V sampling factors */
    (void) read_1_byte(jerr);	/* Quantization table number */
  }
}

LOCAL(void)
process_COM (j_compress_ptr jerr, char *pcomment, int nmaxcommentlen)
{
  unsigned int length;
  int ch;
  int lastch = 0;
	int nlen = 0;

  /* Get the marker parameter length count */
  length = read_2_bytes(jerr);
  /* Length includes itself, so must be at least 2 */
  if (length < 2)
    ERREXIT(jerr,JERR_BAD_LENGTH);
  length -= 2;

  while (length > 0) {
    ch = read_1_byte(jerr);
		nlen = strlen(pcomment);

		if (nlen+1 < nmaxcommentlen)
			pcomment[nlen] = ch;
#if 0
    /* Emit the character in a readable form.
     * Nonprintables are converted to \nnn form,
     * while \ is converted to \\.
     * Newlines in CR, CR/LF, or LF form will be printed as one newline.
     */
    if (ch == '\r') {
			appendstring("\n");
//      printf("\n");
    } else if (ch == '\n') {
      if (lastch != '\r')
				appendstring("\n");
//	printf("\n");
    } else if (ch == '\\') {
			appendstring("\\\\");
//      printf("\\\\");
    } else if (isprint(ch)) {
			appendstring("\n");
      putc(ch, stdout);
    } else {
      printf("\\%03o", ch);
    }
    lastch = ch;
#endif
    length--;
  }
//  printf("\n");
}

/*
 * Parse the marker stream until SOS or EOI is seen;
 * display any COM markers.
 * While the companion program wrjpgcom will always insert COM markers before
 * SOFn, other implementations might not, so we scan to SOS before stopping.
 * If we were only interested in the image dimensions, we would stop at SOFn.
 * (Conversely, if we only cared about COM markers, there would be no need
 * for special code to handle SOFn; we could treat it like other markers.)
 */

LOCAL(int)
read_JPEG_header (j_compress_ptr jerr, char *pcomment, int nmaxcommentlen,
										char *pinfo, int nmaxinfolen)
{
  int marker;
	int nComment = 0;

	memset(pcomment,'\0',nmaxcommentlen);

	if (pinfo)
		memset(pinfo,'\0',nmaxinfolen);
		
  /* Expect SOI at start of file */
  if (first_marker(jerr) != M_SOI)
	{
    ERREXIT(jerr,IWS_NO_SOI);
		return 0;
	}

  /* Scan miscellaneous markers until we reach SOS. */
  for (;;) {
    marker = next_marker(jerr);
    switch (marker) {
      /* Note that marker codes 0xC4, 0xC8, 0xCC are not, and must not be,
       * treated as SOFn.  C4 in particular is actually DHT.
       */
    case M_SOF0:		/* Baseline */
    case M_SOF1:		/* Extended sequential, Huffman */
    case M_SOF2:		/* Progressive, Huffman */
    case M_SOF3:		/* Lossless, Huffman */
    case M_SOF5:		/* Differential sequential, Huffman */
    case M_SOF6:		/* Differential progressive, Huffman */
    case M_SOF7:		/* Differential lossless, Huffman */
    case M_SOF9:		/* Extended sequential, arithmetic */
    case M_SOF10:		/* Progressive, arithmetic */
    case M_SOF11:		/* Lossless, arithmetic */
    case M_SOF13:		/* Differential sequential, arithmetic */
    case M_SOF14:		/* Differential progressive, arithmetic */
    case M_SOF15:		/* Differential lossless, arithmetic */
      if (pinfo)
	process_SOFn(jerr,pinfo,nmaxinfolen,marker);
      else
	skip_variable(jerr);
      break;

    case M_SOS:			/* stop before hitting compressed data */
      return marker;

    case M_EOI:			/* in case it's a tables-only JPEG stream */
      return marker;

    case M_COM:
			if (nComment) {
			 if ((int)strlen(pcomment)+4 < nmaxcommentlen)
				strcat(pcomment,COMMENT_SEPERATOR);
			}
      process_COM(jerr,pcomment,nmaxcommentlen);
			nComment++;
      break;

    case M_APP12:
      /* Some digital camera makers put useful textual information into
       * APP12 markers, so we print those out too when in -verbose mode.
       */
      if (pinfo && ((int)strlen(pinfo)+20 < nmaxinfolen)) { 
		strcat(pinfo,"[APP12] contains:");
		process_COM(jerr,pinfo,nmaxinfolen);
      } else
	skip_variable(jerr);
      break;

    default:			/* Anything else just gets skipped */
      skip_variable(jerr);		/* we assume it has a parameter count... */
      break;
    }
  } /* end loop */
}

GLOBAL(int)
jpeg_add_comment(char *pjpgpath, char *pcomment)
{
	int nRet = 0;
	int infilesize = 0;
	int outfilesize = 0;
  struct jpeg_error_mgr jerr;
  struct jpeg_compress_struct cinfo;
	unsigned int comment_length = 0;
	int marker;
  char *comment_arg = NULL;
	int random = 0;
	char *pMemFile = 0;
	int markerlen = 4; /* length of [COM] marker, prefix and suffix */
	int commentbufferlen = 0;

	if (pcomment)
		comment_length = strlen(pcomment);

	memset(&cinfo,'\0',sizeof(cinfo));

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);

	if (comment_length > 65536)
    ERREXIT(&cinfo, IWS_COMMENT_ERR);

  if ((infile = fopen(pjpgpath, READ_BINARY)) == NULL) {
    ERREXITS(&cinfo, IWS_OPEN_FAILED, pjpgpath);
		return FALSE;
  }
	fseek(infile,0,SEEK_END);
	infilesize = ftell(infile); /* get file size */
	fseek(infile,0,SEEK_SET);

	/* allocate and initialize the memory buffers */
	if (!pjpgmemfile)
		pjpgmemfile = calloc(1,sizeof(JPGMEMFILE));

	if (!pjpgmemfile) {
		fclose(infile);
    ERREXIT(&cinfo, JERR_OUT_OF_MEMORY);
		return 0;
	}
	
	pjpgmemfile->nlen = 0;
	pjpgmemfile->nmaxbuffersize = 0;
	pjpgmemfile->nerrors = 0;

	if (pjpgmemfile->pjpgfile)
		free(pjpgmemfile->pjpgfile);

	pjpgmemfile->pjpgfile = 0;

	commentbufferlen = comment_length + (comment_length ? markerlen : 0);
	pMemFile = calloc(infilesize+commentbufferlen,sizeof(BYTE));

	if (!pMemFile) {
		fclose(infile);
    ERREXIT(&cinfo, JERR_OUT_OF_MEMORY);
		return 0;
	}

	pjpgmemfile->pjpgfile = pMemFile;
	pjpgmemfile->nmaxbuffersize = infilesize+commentbufferlen;

  /* Copy JPEG headers until SOFn marker;
   * we will insert the new comment marker just before SOFn.
   * This (a) causes the new comment to appear after, rather than before,
   * existing comments; and (b) ensures that comments come after any JFIF
   * or JFXX markers, as required by the JFIF specification.
   */
	comment_arg = pcomment;
  marker = scan_JPEG_header(&cinfo,(comment_length ? TRUE : FALSE));
  /* Insert the new COM marker, but only if nonempty text has been supplied */
  if (comment_length > 0) {
    write_marker(M_COM);
    write_2_bytes(comment_length + 2);
    while (comment_length > 0) {
      write_1_byte(*comment_arg++);
      comment_length--;
    }
  }
  /* Duplicate the remainder of the source file.
   * Note that any COM markers occuring after SOF will not be touched.
   */
  write_marker(marker);
  copy_rest_of_file();

	fclose(infile); /* close original file, we're going to overwrite it... */

	if (!pjpgmemfile->nerrors) { /* no errors occurred, overwrite original file */
		if ((infile = fopen(pjpgpath, WRITE_BINARY)) == NULL) {
	    ERREXITS(&cinfo, IWS_WRITE_OPEN_FAILED, pjpgpath);
			return 0;
		}
		if ((int)fwrite(pjpgmemfile->pjpgfile, sizeof(BYTE), pjpgmemfile->nlen, infile) == pjpgmemfile->nlen) {
			nRet = 1;
		}
		else {
	    ERREXITS(&cinfo, IWS_WRITE_FAILED, pjpgpath);
		}
		fclose(infile);
	}
	else
		ERREXIT(&cinfo, IWS_COMMENT_ERROR);

	/* clean up memory based file */
	if (pjpgmemfile->pjpgfile)
		free(pjpgmemfile->pjpgfile);
	if (pjpgmemfile) {
		free(pjpgmemfile);
		pjpgmemfile = 0;
	}

	return nRet;
}

GLOBAL(BOOL)
jpeg_replace_comment(char *pjpgpath, char *pcomment)
{
	BOOL bRet = FALSE;
#if 0

  if ((infile = fopen(argv[argn], READ_BINARY)) == NULL) {
    fprintf(stderr, "%s: can't open %s\n", progname, argv[argn]);
//    exit(EXIT_FAILURE);
		return FALSE;
  }

  /* Copy JPEG headers until SOFn marker;
   * we will insert the new comment marker just before SOFn.
   * This (a) causes the new comment to appear after, rather than before,
   * existing comments; and (b) ensures that comments come after any JFIF
   * or JFXX markers, as required by the JFIF specification.
   */
  marker = scan_JPEG_header(TRUE);
  /* Insert the new COM marker, but only if nonempty text has been supplied */
  if (comment_length > 0) {
    write_marker(M_COM);
    write_2_bytes(comment_length + 2);
    while (comment_length > 0) {
      write_1_byte(*comment_arg++);
      comment_length--;
    }
  }
  /* Duplicate the remainder of the source file.
   * Note that any COM markers occuring after SOF will not be touched.
   */
  write_marker(marker);
  copy_rest_of_file();
#endif//
	return bRet;
}

GLOBAL(BOOL)
jpeg_get_comment(char *pjpgpath, char *pcomment, int ncommentlen,
										char *pjpginfo, int ninfolen)
{
	BOOL bRet = FALSE;
  struct jpeg_error_mgr jerr;
  struct jpeg_compress_struct cinfo;
	unsigned int comment_length = 0;
  char *comment_arg = NULL;

	memset(&cinfo,'\0',sizeof(cinfo));

  /* Initialize the JPEG compression object with default error handling. */
  cinfo.err = jpeg_std_error(&jerr);

  if ((infile = fopen(pjpgpath, READ_BINARY)) == NULL) {
    ERREXITS(&cinfo, IWS_OPEN_FAILED, pjpgpath);
		return FALSE;
  }

  /* Scan the JPEG headers. */
  read_JPEG_header(&cinfo,pcomment,ncommentlen,pjpginfo,ninfolen);

	bRet = TRUE;

	fclose(infile);

	return bRet;
}
