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

#ifndef _UTIL_H
#define _UTIL_H

/* UPDATED: 03/15/2005 by MDG */

#ifndef True
#define True	1
#define False	0
#endif

/* bres.c */
extern int bres_line_alloc(const int, const int, const int, const int, int **,
           int **, int *, int *);

/* bubble.c */
extern void bubble_sort_int(int *, const int);

/* fatalerr.c */
extern void fatalerr(char *, char *, char *);

/* invbytes.h */
extern void inv_bytes(unsigned char *, int);

/* ssxstats.c */
extern double ssx_stddev(const double, const double, const int);
extern double ssx_variance(const double, const double, const int);
extern double ssx(const double, const double, const int);

/* syserr.c */
extern void syserr(char *, char *, char *);

/* ticks.c */
extern unsigned long ticks(void);
extern int ticksPerSec(void);

/* time.c */
extern char *current_time(void);

#endif /* !_UTIL_H */
