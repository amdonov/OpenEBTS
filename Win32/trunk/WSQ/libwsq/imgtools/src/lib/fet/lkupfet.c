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
      LIBRARY: FET - Feature File/List Utilities

      FILE:    LKUPFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001

      Contains routines responsible for looking up the value of
      a specified attribute in and attribute-value paired list.

      ROUTINES:
#cat: lookupfet - returns the specified feature entry from an fet
#cat:             structure.  Returns TRUE if found, FALSE if not.

***********************************************************************/

#include <usebsd.h>
#include <string.h>
#include <fet.h>
#include <defs.h>

/*******************************************************************/
int lookupfet(char **ovalue, char *feature, FET *fet)
{
  int item;
  char *value;

  for (item = 0;
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if (item>=fet->num){
     return(FALSE);
  }
  if(fet->values[item] != (char *)NULL){
      value = strdup(fet->values[item]);
      if (value == (char *)NULL){
         fprintf(stderr, "ERROR : lookupfet : strdup : value\n");
         return(-2);
     }
  }
  else
      value = (char *)NULL;

  *ovalue = value;

  return(TRUE);
}
