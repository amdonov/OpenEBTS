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

      FILE:    STRFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001
      UPDATED: 03/10/2005 by MDG

      Contains routines responsible for converting an attribute-value
      paired list to and from a null-terminated string.

      ROUTINES:
#cat: fet2string - takes an FET structure and concatenates (name,value)
#cat:              pairs into a single null-terminated string with each
#cat:              (name,value) pair delimited by a new-line.
#cat: string2fet - parses a null-terminated string representing a
#cat:              list of (name,value) pairs into an FET structure.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <fet.h>

/*****************************************************************/
int fet2string(char **ostr, FET *fet)
{
   int i, size;
   char *str;

   /* Calculate size of string. */
   size = 0;
   for(i = 0; i < fet->num; i++){
      size += strlen(fet->names[i]);
      size += strlen(fet->values[i]);
      size += 2;
   }
   /* Make room for NULL for final strlen() below. */
   size++;

   if((str = (char *)calloc(size, sizeof(char))) == (char *)NULL){
      fprintf(stderr, "ERROR : fet2string : malloc : str\n");
      return(-2);
   }

   for(i = 0; i < fet->num; i++){
      strcat(str, fet->names[i]);
      strcat(str, " ");
      strcat(str, fet->values[i]);
      strcat(str, "\n");
   }

   str[strlen(str)-1] = '\0';

   *ostr = str;
   return(0);
}

/*****************************************************************/
int string2fet(FET **ofet, char *istr)
{
   int ret;
   char *iptr, *optr;
   char name[MAXFETLENGTH], value[MAXFETLENGTH], *vptr;
   FET *fet;

   if((ret = allocfet_ret(&fet, MAXFETS)))
      return(ret);

   iptr = istr;
   while(*iptr != '\0'){
      /* Get next name */
      optr = name;
      while((*iptr != '\0')&&(*iptr != ' ')&&(*iptr != '\t'))
         *optr++ = *iptr++;
      *optr = '\0';

      /* Skip white space */
      while((*iptr != '\0')&&
            ((*iptr == ' ')||(*iptr == '\t')))
         iptr++;

      /* Get next value */
      optr = value;
      while((*iptr != '\0')&&(*iptr != '\n'))
         *optr++ = *iptr++;
      *optr = '\0';

      /* Skip white space */
      while((*iptr != '\0')&&
            ((*iptr == ' ')||(*iptr == '\t')||(*iptr == '\n')))
         iptr++;

      /* Test (name,value) pair */
      if(strlen(name) == 0){
         fprintf(stderr, "ERROR : string2fet : empty name string found\n");
         return(-2);
      }
      if(strlen(value) == 0)
         vptr = (char *)NULL;
      else
         vptr = value;

      /* Store name and value pair into FET. */
      if((ret = updatefet_ret(name, vptr, fet))){
         freefet(fet);
         return(ret);
      }
   }

   *ofet = fet;
   return(0);
}
