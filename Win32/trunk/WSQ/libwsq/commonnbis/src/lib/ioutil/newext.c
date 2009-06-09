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

      FILE:    NEWEXT.C
      AUTHOR:  Michael Garris
      DATE:    11/27/89
      UPDATED: 03/11/2005 by MDG

      Contains routines responsible for replacing a file name's
      extension with a new one.

      ROUTINES:
#cat: newext - takes a filename, strips off the rightmost extenstion, and
#cat:          appends a new extension.  This routine exits upon error.
#cat: newext_ret - takes a filename, strips off the rightmost extenstion, and
#cat:          appends a new extension.  This routine returns an error code.
#cat: newextlong - takes a pointer to a filename, strips the rightmost
#cat:		 extenstion, and appends an arbitrary new extension.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

/*************************************************************/
/* Newext() is a destructive procedure which takes a filename*/
/* and strips off the rightmost extension (if one exists) and*/
/* appends the extension passed.  Exits upon error.          */
/*************************************************************/
void newext(char *file, const int len, char *ext)
{
   char *cptr;

   cptr = file + strlen(file);
   while((cptr != file) && (*cptr != '.'))
      cptr--;
   if(cptr == file){
      if(strlen(file) == len)
         fatalerr("newext","File manipulation exceeds allocated memory",NULL);
      cptr += strlen(file);
      *cptr++ = '.';
      /* EDIT MDG 1/25/99
      *cptr = NULL;
      */
      *cptr = '\0';
   }
   else{
      cptr++;
      /* EDIT MDG 1/25/99
      *cptr = NULL;
      */
      *cptr = '\0';
   }
   if(strlen(file) + strlen(ext) > len)
      fatalerr("newext", file, "proposed extension too long");

   strcat(file,ext);
}

/*************************************************************/
/* Newext_ret() is a destructive procedure, taking a filename*/
/* and striping off the rightmost extension (if one exists)  */
/* & appends the extension passed.  Returns code upon error. */
/*************************************************************/
int newext_ret(char *file, int len, char *ext)
{
   char *cptr;

   cptr = file + strlen(file);
   while((cptr != file) && (*cptr != '.'))
      cptr--;
   if(cptr == file){
      if(strlen(file) == len){
         fprintf(
         stderr, "ERROR : newext_ret: file manipulation exceeds memory\n");
         return(-2);
      }
      cptr += strlen(file);
      *cptr++ = '.';
      *cptr = (char)NULL;
   }
   else{
      cptr++;
      *cptr = (char)NULL;
   }
   if(strlen(file) + strlen(ext) > len){
      fprintf(stderr, "ERROR : newext_ret : proposed extension too long\n");
      return(-3);
   }

   strcat(file,ext);

   return(0);
}

/*************************************************************/
void newextlong(char **file, char *ext)
{
int n, m;
char *cptr, *tmp;

   n = strlen(*file);      /* the original length                     */
   m = strlen(ext);

   cptr = *file + n;
   while((cptr != *file) && (*cptr != '.'))
      cptr--;

   if (cptr == *file)      /* add an extension where there wasn't one */
   {
      n += m;
      if ((tmp = (char *)calloc(n+1, sizeof(char))) == NULL)
         syserr("newextlong", "calloc", "space for new string result");
      sprintf(tmp, "%s.%s", *file, ext);
      free(*file);
      *file = tmp;
   }
   else
   {                      /* replace the existing extension          */
      cptr++;
      /* EDIT MDG 1/25/99
      *cptr = NULL;
      */
      *cptr = '\0';
      if ((cptr - *file) + m > n)
      {
         n += m;
         if ((tmp = (char *)calloc(n+1, sizeof(char))) == NULL)
            syserr("newextlong", "calloc", "space for new string result");
         sprintf(tmp, "%s%s", *file, ext);
         free(*file);
         *file = tmp;
      }
      else
         strcat(*file, ext);
   }
}
