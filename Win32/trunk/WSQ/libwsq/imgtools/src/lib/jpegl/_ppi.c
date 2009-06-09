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

/***********************************************************************
      LIBRARY: JPEGL - Lossless JPEG Image Compression

      FILE:    PPI.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    01/17/2001

      Contains routines responsible for determining the scan resolution
      in units of pixels per inch from a JPEGL compressed datastream.

      ROUTINES:
#cat: get_ppi_jpegl - Given a JFIF Header from a JPEGL compressed
#cat:                 datastream, extracts/derives the pixel scan
#cat:                 resolution in units of pixel per inch.

***********************************************************************/

#include <stdio.h>
#include <jpegl.h>

#define CM_PER_INCH   2.54

/************************************************************************/
int get_ppi_jpegl(int *oppi, JFIF_HEADER *jfif_header)
{
   int ppi;

   /* Get and set scan density in pixels per inch. */
   switch(jfif_header->units){
      /* pixels per inch */
      case 1:
         /* take the horizontal pixel density, even if the vertical is */
         /* not the same */
         ppi = jfif_header->dx;
         break;
      /* pixels per cm */
      case 2:
         /* compute ppi from horizontal density even if not */
         /* equal to vertical */
         ppi = (int)((jfif_header->dx * CM_PER_INCH) + 0.5);
         break;
      /* unknown density */
      case 0:
         /* set ppi to -1 == UNKNOWN */
         ppi = -1;
         break;
      /* ERROR */
      default:
         fprintf(stderr, "ERROR : get_ppi_jpegl : ");
         fprintf(stderr, "illegal density unit = %d\n", jfif_header->units);
         return(-2);
   }

   *oppi = ppi;

   return(0);
}
