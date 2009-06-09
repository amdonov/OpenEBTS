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
      LIBRARY: IOUTIL - INPUT/OUTPUT Utilities

      FILE:    FILEROOT.C
      AUTHOR:  Michael Garris
      DATE:    11/27/1989
      UPDATED: 03/15/2005 by MDG

      Contains routines responsible for extracting the root name
      from a file name string.

      ROUTINES:
#cat: fileroot - takes a file name and strips off the rightmost extension,
#cat:            if one is present.

***********************************************************************/

#include <stdio.h>
#include <string.h>

/*************************************************************/
/* Fileroot() is a destructive procedure which takes a file  */
/* name and strips off the rightmost extension, if it exists.*/
/*************************************************************/
void fileroot(char *file)
{
   char *cptr;

   cptr = file + strlen(file);
   while((cptr != file) && (*cptr != '.'))
      cptr--;
   if(*cptr == '.')
     /* EDIT MDG 1/25/99
      *cptr = NULL;
     */
      *cptr = '\0';
}

