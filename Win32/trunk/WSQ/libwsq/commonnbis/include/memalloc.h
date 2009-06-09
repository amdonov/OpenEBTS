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

#ifndef _MEMALLOC_H
#define _MEMALLOC_H

/* UPDATED: 03/15/2005 by MDG */

extern int malloc_char_ret(char **, const int, char *);
extern int malloc_uchar_ret(unsigned char **, const int, char *);
extern int malloc_int_ret(int **, const int, char *);
extern int calloc_int_ret(int **, const int, char *);
extern int realloc_int_ret(int **, const int, char *);

extern void *datadup(void *, int, char *);
extern void malloc_char(char **, int, char *);
extern void malloc_uchar(unsigned char **, int, char *);
extern void malloc_shrt(short **, int, char *);
extern void malloc_int(int **, int, char *);
extern void malloc_flt(float **, int, char *);
extern void calloc_char(char **, int, char *);
extern void calloc_uchar(unsigned char **, int, char *);
extern void calloc_shrt(short **, int, char *);
extern void calloc_int(int **, int, char *);
extern void calloc_flt(float **, int, char *);
extern void malloc_dbl_char_l1(char ***, int, char *);
extern void malloc_dbl_uchar_l1(unsigned char ***, int, char *);
extern void malloc_dbl_shrt_l1(short ***, int, char *);
extern void malloc_dbl_int_l1(int ***, int, char *);
extern void malloc_dbl_flt_l1(float ***, int, char *);
extern void realloc_char(char **, int, char *);
extern void realloc_uchar(unsigned char **, int, char *);
extern void realloc_shrt(short **, int, char *);
extern void realloc_int(int **, int, char *);
extern void realloc_flt(float **, int, char *);
extern void realloc_dbl_int_l1(int ***, int, char *);
extern void realloc_dbl_char_l1(char ***, int, char *);
extern void realloc_dbl_uchar_l1(unsigned char ***, int, char *);
extern void realloc_dbl_flt_l1(float ***, int, char *);
extern void free_dbl_char(char **, const int);
extern void free_dbl_uchar(unsigned char **, const int);
extern void free_dbl_flt(float **, const int);
extern void malloc_dbl_char(char ***, const int, const int, char *);
extern void malloc_dbl_uchar(unsigned char ***, const int, const int, char *);
extern void malloc_dbl_flt(float ***, const int, const int, char *);

#endif /* !_MEMALLOC_H */
