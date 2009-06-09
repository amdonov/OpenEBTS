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

#ifndef _MTCH_PNM_H
#define _MTCH_PNM_H

/* Names of the values of the a_type parm of mtch_pnm. */
#define MP_FILENAME ((char)0)
#define MP_INT      ((char)1)
#define MP_FLOAT    ((char)2)
#define MP_SWITCH   ((char)3)

/* Bundles together some parms for mtch_pnm, to reduce the verbosity
of the (many) calls of it by st_nv_ok. */
typedef struct {
  char *namestr, *valstr, *errstr, ok;
  int linenum;
} NVEOL;

#endif /* !_MTCH_PNM_H */
