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

#ifndef _RGB_YCC_H
#define _RGB_YCC_H

#ifndef MAX_CMPNTS
#define MAX_CMPNTS   4
#endif

#ifndef sround
#define sround(x) ((int) (((x)<0) ? (x)-0.5 : (x)+0.5))
#endif

extern int rgb2ycc_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int, const int);
extern int rgb2ycc_intrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int rgb2ycc_nonintrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int downsample_cmpnts(unsigned char **, int *, unsigned char *,
                       const int, const int, const int,
                       int *, int *, const int);
extern void window_avr_plane(unsigned char *, int *, int *, const int,
                       const int, unsigned char *, const int, const int);
extern int avr_window(unsigned char *, const int, const int, const int,
                       const int);
extern int ycc2rgb_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int, const int);
extern int ycc2rgb_intrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int ycc2rgb_nonintrlv_mem(unsigned char **, int *, unsigned char *,
                       const int, const int, const int);
extern int upsample_cmpnts(unsigned char **, int *, unsigned char *,
                       const int, const int, const int,
                       int *, int *, const int);
extern void window_fill_plane(unsigned char *, const int, const int,
                       const int, const int,
                       unsigned char *, const int, const int);
extern void fill_window(const unsigned char, unsigned char *,
                       const int, const int, const int, const int);
extern int test_evenmult_sampfctrs(int *, int *, int *, int *, const int);

#endif /* !_RGB_YCC_H */
