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
      LIBRARY: UTIL - General Purpose Utility Routines

      FILE:    INVBYTES.C
      AUTHOR:  Michael Garris
      DATE:    10/16/1995
      UPDATED: 04/25/2005 by MDG

      Contains routines responsible for reversing the bits within a
      data byte from MSB to LSB and vice versa.

      ROUTINES:
#cat: inv_bytes - takes an array of bytes and inverts them using a
#cat:             lookup table. invert = reverse bits within a byte

***********************************************************************/
#include <invbyte.h>

void inv_bytes(unsigned char *data, int nbytes)
{
   for ( ; nbytes ; nbytes--, data++ )
      *data = Inv_Byte[*data];
}
