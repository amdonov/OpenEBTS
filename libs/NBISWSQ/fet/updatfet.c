/*******************************************************************************

License: 
This software was developed at the National Institute of Standards and 
Technology (NIST) by employees of the Federal Government in the course 
of their official duties. Pursuant to title 17 Section 105 of the 
United States Code, this software is not subject to copyright protection 
and is in the public domain. NIST assumes no responsibility  whatsoever for 
its use by other parties, and makes no guarantees, expressed or implied, 
about its quality, reliability, or any other characteristic. 

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain.  Therefore,
it is permissible to distribute this software as a free download from the
internet.

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

      FILE:    UPDATFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001
      UPDATED: 03/10/2005 by MDG
	           02/28/2007 by Kenneth Ko

      Contains routines responsible for replacing the value of
      a specified attribute in an attribute-value paird list.

      ROUTINES:
#cat: updatefet - replaces a feature entry in an fet structure, or creates
#cat:             a new entry if the feature does not already exist.
#cat:             Exits on error.
#cat: updatefet_ret - replaces a feature entry in an fet structure, or
#cat:             creates a new entry if the feature does not already exist.
#cat:             Returns on error.

***********************************************************************/

#include <usebsd.h>
#include <string.h>
#ifndef WIN32			//DJD
#include <strings.h>	//DJD
#else					//DJD
#define strdup _strdup	//DJD
#endif					//DJD
#include <fet.h>
#include <defs.h>
#include <util.h>

/***********************************************************************/
void updatefet(char *feature, char *value, FET *fet)
{
  int item;
  int increased, incr;

  for (item = 0;
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if (item < fet->num){
     if(fet->values[item] != (char *)NULL){
        free(fet->values[item]);
        fet->values[item] = (char *)NULL;
     }
     if(value != (char *)NULL){
        fet->values[item] = strdup(value);
        if(fet->values[item] == (char *)NULL)
           syserr("updatefet","strdup","fet->values[]");
     }
  }
  else{
     if(fet->num >= fet->alloc){
        incr      = fet->alloc / 10;		/* add 10% or 10 which-	*/
        increased = fet->alloc + max(10, incr);	/* ever is larger	*/
        reallocfet(fet, increased);
     }
     fet->names[fet->num] = (char *)strdup(feature);
     if(fet->names[fet->num] == (char *)NULL)
        syserr("updatefet","strdup","fet->names[]");
     if(value != (char *)NULL){
        fet->values[fet->num] = (char *)strdup(value);
        if(fet->values[fet->num] == (char *)NULL)
           syserr("updatefet","strdup","fet->values[]");
     }
     (fet->num)++;
  }
}

/***********************************************************************/
int updatefet_ret(char *feature, char *value, FET *fet)
{
  int ret, item;
  int increased, incr;

  for (item = 0;
       (item < fet->num) && (strcmp(fet->names[item],feature) != 0);
       item++);
  if (item < fet->num){
     if(fet->values[item] != (char *)NULL){
        free(fet->values[item]);
        fet->values[item] = (char *)NULL;
     }
     if(value != (char *)NULL){
        fet->values[item] = (char *)strdup(value);
        if(fet->values[item] == (char *)NULL){
           fprintf(stderr, "ERROR : updatefet_ret : strdup : fet->values[]\n");
           return(-2);
        }
     }
  }
  else{
     if(fet->num >= fet->alloc){
        incr      = fet->alloc / 10;		/* add 10% or 10 which-	*/
        increased = fet->alloc + max(10, incr);	/* ever is larger	*/
        if((ret = reallocfet_ret(&fet, increased)))
           return(ret);
     }
     fet->names[fet->num] = (char *)strdup(feature);
     if(fet->names[fet->num] == (char *)NULL){
        fprintf(stderr, "ERROR : updatefet_ret : strdup : fet->names[]\n");
        return(-3);
     }
     if(value != (char *)NULL){
        fet->values[fet->num] = (char *)strdup(value);
        if(fet->values[fet->num] == (char *)NULL){
           fprintf(stderr, "ERROR : updatefet_ret : strdup : fet->values[]\n");
           return(-4);
        }
     }
     (fet->num)++;
  }

  return(0);
}
