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

      FILE:    FILEHEAD.C
      AUTHOR:  Michael Garris
      DATE:    11/27/1989
      UPDATED: 03/15/2005 by MDG

      Contains routines responsible for extracing the "head" of
      a the directory path in a file name string.

      ROUTINES:
#cat: filehead - takes a full pathname and returns the information to the
#cat:            left of the rightmost directory level.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <util.h>

/*************************************************************/
/* Filehead() is a destructive procedure which takes a path  */
/* name and returns all directory information to the left    */
/* of the rightmost directory level.                         */
/*************************************************************/
void filehead(char *head)
{

   char *cptr;

   cptr = head + strlen(head);
   while((cptr != head) && (*cptr != '/'))
      cptr--;
   if(cptr != head)
     /* EDIT MDG 1/25/99
      *cptr = NULL;
     */
      *cptr = '\0';
   else
      fatalerr("filehead","full path required as input",NULL);
}
