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

#ifndef _IMGDECOD_H
#define _IMGDECOD_H

#include <imgtype.h>

#define IMG_IGNORE  2

extern int read_and_decode_grayscale_image(char *, int *, unsigned char **,
                                    int *, int *, int *, int *, int *);

extern int read_and_decode_dpyimage(char *, int *, unsigned char **, int *,
                                    int *, int *, int *, int *);

extern int read_and_decode_image(char *, int *, unsigned char **, int *,
                                 int *, int *, int *, int *, int *,
                                 int *, int *, int *);

extern int ihead_decode_mem(unsigned char **, int *, int *, int *,
                            int *, int *, unsigned char *, const int);

#endif /* !_IMGDECOD_H */
