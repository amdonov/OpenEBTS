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

      FILE:    READFET.C
      AUTHOR:  Michael Garris
      DATE:    01/11/2001
      UPDATED: 03/10/2005 by MDG

      Contains routines responsible for reading the contents of
      a file into a data structure holding an attribute-value
      paired list.

      ROUTINES:
#cat: readfetfile - opens an fet file and reads its contents into an
#cat:               fet structure.  Exits on error.
#cat: readfetfile_ret - opens an fet file and reads its contents into an
#cat:               fet structure.  Returns on error.

***********************************************************************/

#include <usebsd.h>
#include <string.h>
#include <fet.h>
#include <util.h>

/*****************************************************************/
FET *readfetfile(char *file)
{
   FILE *fp;
   FET *fet;
   char c,buf[MAXFETLENGTH];

   if ((fp = fopen(file,"rb")) == (FILE *)NULL)
      syserr("readfetfile","fopen",file);

   fet = allocfet(MAXFETS);
   while (fscanf(fp,"%s",buf) != EOF){
      while(((c = getc(fp)) == ' ') || (c == '\t'));
      ungetc(c, fp);
      if (fet->num >= fet->alloc)
         reallocfet(fet, fet->alloc + MAXFETS);
      fet->names[fet->num] = strdup(buf);
      if(fet->names[fet->num] == (char *)NULL)
         syserr("readfetfile","strdup","fet->names[]");
      fgets(buf,MAXFETLENGTH-1,fp);
      buf[strlen(buf)-1] = '\0';
      fet->values[fet->num] = (char *)strdup(buf);
      if(fet->values[fet->num] == (char *)NULL)
         syserr("readfetfile","strdup","fet->values[]");
      (fet->num)++;
   }
   fclose(fp);
   return(fet);
}

/*****************************************************************/
int readfetfile_ret(FET **ofet, char *file)
{
   int ret;
   FILE *fp;
   FET *fet;
   char c,buf[MAXFETLENGTH];

   if ((fp = fopen(file,"rb")) == (FILE *)NULL){
      fprintf(stderr, "ERROR : readfetfile_ret : fopen : %s\n", file);
      return(-2);
   }

   if((ret = allocfet_ret(&fet, MAXFETS))){
      fclose(fp);
      return(ret);
   }

   while (fscanf(fp,"%s",buf) != EOF){
      while(((c = getc(fp)) == ' ') || (c == '\t'));
      ungetc(c, fp);
      if (fet->num >= fet->alloc){
         if((ret = reallocfet_ret(&fet, fet->alloc + MAXFETS))){
            fclose(fp);
            freefet(fet);
            return(ret);
         }
      }
      fet->names[fet->num] = (char *)strdup(buf);
      if(fet->names[fet->num] == (char *)NULL){
         fprintf(stderr, "ERROR : readfetfile_ret : strdup : fet->names[]\n");
         fclose(fp);
         freefet(fet);
         return(-3);
      }
      fgets(buf,MAXFETLENGTH-1,fp);
      buf[strlen(buf)-1] = '\0';
      fet->values[fet->num] = (char *)strdup(buf);
      if(fet->values[fet->num] == (char *)NULL){
         fprintf(stderr, "ERROR : readfetfile_ret : strdup : fet->values[]\n");
         fclose(fp);
         freefet(fet);
         return(-4);
      }
      (fet->num)++;
   }
   fclose(fp);
   *ofet = fet;

   return(0);
}
