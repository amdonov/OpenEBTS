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

      FILE:    ALLOCFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001
      UPDATED: 03/10/2005 by MDG

      Contains routines responsibile allocating data structures
      used to hold attribute-value paired lists.

      ROUTINES:
#cat: allocfet - allocates and initialized an empty fet structure.
#cat:            Exits on error.
#cat: allocfet_ret - allocates and initialized an empty fet structure.
#cat:            Returns on error.
#cat: reallocfet - reallocates an fet structure of a
#cat:            specified length.  Exits on error.
#cat: reallocfet_ret - reallocates an fet structure of a
#cat:            specified length.  Returns on error.

***********************************************************************/

#include <stdio.h>
#include <fet.h>
#include <util.h>

/********************************************************************/
FET *allocfet(int numfeatures)
{
   FET *fet;

   fet = (FET *)malloc(sizeof(FET));
   if (fet == (FET *)NULL)
      syserr("allocfet","malloc","fet");
   /* calloc here is required */
   fet->names = (char **)calloc(numfeatures, sizeof(char *));
   if (fet->names == (char **)NULL)
      syserr("allocfet","calloc","fet->names");
   fet->values = (char **)calloc(numfeatures, sizeof(char *));
   if (fet->values == (char **)NULL)
      syserr("allocfet","calloc","fet->values");
   fet->alloc = numfeatures;
   fet->num = 0;
   return(fet);
}

/********************************************************************/
int allocfet_ret(FET **ofet, int numfeatures)
{
   FET *fet;

   fet = (FET *)malloc(sizeof(FET));
   if (fet == (FET *)NULL){
      fprintf(stderr, "ERROR : allocfet_ret : malloc : fet\n");
      return(-2);
   }
   /* calloc here is required */
   fet->names = (char **)calloc(numfeatures, sizeof(char *));
   if (fet->names == (char **)NULL){
      fprintf(stderr, "ERROR : allocfet_ret : calloc : fet->names\n");
      free(fet);
      return(-3);
   }
   fet->values = (char **)calloc(numfeatures, sizeof(char *));
   if (fet->values == (char **)NULL){
      fprintf(stderr, "ERROR : allocfet_ret : calloc : fet->values\n");
      free(fet->names);
      free(fet);
      return(-4);
   }
   fet->alloc = numfeatures;
   fet->num = 0;

   *ofet = fet;

   return(0);
}

/********************************************************************/
FET *reallocfet(FET *fet, int newlen)
{
   if (fet == (FET *)NULL || fet->alloc == 0)
      return(allocfet(newlen));

   fet->names = (char **)realloc(fet->names, newlen * sizeof(char *));
   if (fet->names == (char **)NULL)
      fatalerr("reallocfet", "realloc", "space for increased fet->names");
   fet->values = (char **)realloc(fet->values, newlen * sizeof(char *));
   if (fet->values == (char **)NULL)
      fatalerr("reallocfet", "realloc", "space for increased fet->values");
   fet->alloc = newlen;

   return(fet);
}

/********************************************************************/
int reallocfet_ret(FET **ofet, int newlen)
{
   int ret;
   FET *fet;

   fet = *ofet;

   /* If fet not allocated ... */
   if ((fet == (FET *)NULL || fet->alloc == 0)){
      /* Allocate the fet. */
      if((ret = allocfet_ret(ofet, newlen)))
         /* Return error code. */
         return(ret);
      /* Otherwise allocation was successful. */
      return(0);
   }

   /* Oherwise, reallocate fet. */
   fet->names = (char **)realloc(fet->names, newlen * sizeof(char *));
   if (fet->names == (char **)NULL){
      fprintf(stderr, "ERROR : reallocfet_ret : realloc : fet->names\n");
      return(-2);
   }
   fet->values = (char **)realloc(fet->values, newlen * sizeof(char *));
   if (fet->values == (char **)NULL){
      fprintf(stderr, "ERROR : reallocfet_ret : realloc : fet->values");
      return(-3);
   }
   fet->alloc = newlen;

   return(0);
}
