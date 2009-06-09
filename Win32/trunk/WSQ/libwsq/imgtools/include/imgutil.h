/*******************************************************************************

License: 
This software was developed at the National Institute of Standards and 
Technology (NIST) by employees of the Federal Government in the course 
of their official duties. Pursuant to title 17 Section 105 of the 
United States Code, this software is not subject to copyright protection 
and is in the public domain. NIST assumes no responsibility  whatsoever for 
its use by other parties, and makes no guarantees, expressed or implied, 
about its quality, reliability, or any other characteristic. 

Disclaimer: 
This software was developed to promote biometric standards and biometric
technology testing for the Federal Government in accordance with the USA
PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
Specific hardware and software products identified in this software were used
in order to perform the software development.  In no case does such
identification imply recommendation or endorsement by the National Institute
of Standards and Technology, nor does it imply that the products and equipment
identified are necessarily the best available for the purpose.  

*******************************************************************************/

#ifndef _IMGUTIL_H
#define _IMGUTIL_H


/* imageops.c */
extern int WordAlignImage(unsigned char **, int *, int *,
                          unsigned char *, int, int, int);

/* imgsnip.c */
extern void snip_rot_subimage(unsigned char *, const int, const int,
                              unsigned char *, const int, const int, const int,
                              const int, const float, unsigned char);
extern void snip_rot_subimage_interp(unsigned char *, const int, const int,
                              unsigned char *, const int, const int, const int,
                              const int, const float, unsigned char);

/* imgutil.c */
extern void init_image_data(unsigned char **, int *, int, int, int);
extern float PixPerByte(int);
extern int SizeFromDepth(int, int, int);
extern int WordAlignFromDepth(int, int);
extern unsigned char *allocate_image(int, int, int);
extern unsigned char *mallocate_image(int, int, int);
extern short *alloc_short_image(int, int);
extern unsigned char *alloc_char_image(int, int);
extern int *alloc_int_image(int, int);
extern int allocate_aligned_image(unsigned char **, int *, int, int, int);
extern int width_16(int);

/* rl.c */
extern void rlcomp(unsigned char *, int, unsigned char *, int *, int);
extern void PutNchar (long, int, unsigned char **, int *, int);
extern void rldecomp(unsigned char *, int, unsigned char *, int *, int);
extern void RLL_putc (unsigned char **, unsigned char, int, int *);

#endif /* !_IMGUTIL_H */
