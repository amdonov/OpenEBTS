/*******************************************************************************

License: 
This software was developed at the National Institute of Standards and 
Technology (NIST) by employees of the Federal Government in the course 
of their official duties. Pursuant to title 17 Section 105 of the 
United States Code, this software is not subject to copyright protection 
and is in the public domain. NIST assumes no responsibility  whatsoever for 
its use by other parties, and makes no guarantees, expressed or implied, 
about its quality, reliability, or any other characteristic. 

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain.  Therefore,
it is permissible to distribute this software as a free download from the
internet.

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


#ifndef _FET_H
#define _FET_H

#include <stdio.h>
#include <stdlib.h> /* Added by MDG on 03-10-05 */

#ifndef True
#define True		1
#define False		0
#endif
#define FET_EXT		"fet"
#define MAXFETS		100
#define MAXFETLENGTH	512

typedef struct fetstruct{
   int alloc;
   int num;
   char **names;
   char **values;
} FET;

/* allocfet.c */
extern FET  *allocfet(int);
extern int  allocfet_ret(FET **, int);
extern FET  *reallocfet(FET *, int);
extern int  reallocfet_ret(FET **, int);
/* delfet.c */
extern void deletefet(char *, FET *);
extern int  deletefet_ret(char *, FET *);
/* extfet.c */
extern char *extractfet(char *, FET *);
extern int  extractfet_ret(char **, char *, FET *);
/* freefet.c */
extern void freefet(FET *);
/* lkupfet.c */
extern int  lookupfet(char **, char *, FET *);
/* printfet.c */
extern void printfet(FILE *, FET *);
/* readfet.c */
extern FET  *readfetfile(char *);
extern int  readfetfile_ret(FET **, char *);
/* strfet.c */
extern int fet2string(char **, FET *);
extern int string2fet(FET **, char *);
/* updatfet.c */
extern void updatefet(char *, char *, FET *);
extern int  updatefet_ret(char *, char *, FET *);
/* writefet.c */
extern void writefetfile(char *, FET *);
extern int  writefetfile_ret(char *, FET *);

#endif  /* !_FET_H */
