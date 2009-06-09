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

      FILE:    EXTRFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001
      UPDATED: 03/10/2005 by MDG

      Contains routines responsible for locating and returning the
      value stored with a specified attribute in an attribute-value
      paired list.

      ROUTINES:
#cat: extractfet - returns the specified feature entry from an fet structure.
#cat:              Exits on error.
#cat: extractfet_ret - returns the specified feature entry from an fet
#cat:              structure.  Returns on error.

***********************************************************************/

#include <usebsd.h>
#include <string.h>
#include <fet.h>
#include <util.h>


/*******************************************************************/
char *extractfet(char *feature, FET *fet)
{
  int item;
  char *value;

  for (item = 0;
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if (item>=fet->num)
     fatalerr("extractfet",feature,"not found");
  if(fet->values[item] != (char *)NULL){
      value = strdup(fet->values[item]);
      if (value == (char *)NULL)
         syserr("extractfet","strdup","value");
  }
  else
      value = (char *)NULL;
  return(value);
}

/*******************************************************************/
int extractfet_ret(char **ovalue, char *feature, FET *fet)
{
  int item;
  char *value;

  for (item = 0;
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if (item>=fet->num){
     fprintf(stderr, "ERROR : extractfet_ret : feature %s not found\n",
             feature);
     return(-2);
  }
  if(fet->values[item] != (char *)NULL){
      value = (char *)strdup(fet->values[item]);
      if (value == (char *)NULL){
         fprintf(stderr, "ERROR : extractfet_ret : strdup : value\n");
         return(-3);
     }
  }
  else
      value = (char *)NULL;

  *ovalue = value;

  return(0);
}
