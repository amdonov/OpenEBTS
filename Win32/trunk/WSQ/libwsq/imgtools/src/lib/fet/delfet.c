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

      FILE:    DELFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001
      UPDATED: 03/10/2005 by MDG

      Contains routines responsible for removing an entry from an
      attribute-value paired list.

      ROUTINES:
#cat: deletefet - removes the specified feature entry from an fet structure.
#cat:             Exits on error.
#cat: deletefet_ret - removes the specified feature entry from an fet
#cat:             structure.  Returns on error.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <fet.h>
#include <util.h>

/*********************************************************************/
void deletefet(char *feature, FET *fet)
{
  int item;

  for (item = 0; 
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if(item >= fet->num)
     fatalerr("deletefet",feature,"Feature not found");
  free(fet->names[item]);
  if(fet->values[item] != (char *)NULL)
     free(fet->values[item]);
  for (++item;item<fet->num;item++){
      fet->names[item-1] = fet->names[item];
      fet->values[item-1] = fet->values[item];
  }
  fet->names[fet->num-1] = '\0';
  fet->values[fet->num-1] = '\0';
  (fet->num)--;
}

/*********************************************************************/
int deletefet_ret(char *feature, FET *fet)
{
  int item;

  for (item = 0; 
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if(item >= fet->num){
    fprintf(stderr, "ERROR : deletefet_ret : feature %s not found\n",
            feature);
     return(-2);
  }
  free(fet->names[item]);
  if(fet->values[item] != (char *)NULL)
     free(fet->values[item]);
  for (++item;item<fet->num;item++){
      fet->names[item-1] = fet->names[item];
      fet->values[item-1] = fet->values[item];
  }
  fet->names[fet->num-1] = '\0';
  fet->values[fet->num-1] = '\0';
  (fet->num)--;

  return(0);
}
