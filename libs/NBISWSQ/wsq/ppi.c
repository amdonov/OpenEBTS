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
      LIBRARY: WSQ - Grayscale Image Compression

      FILE:    PPI.C
      AUTHORS: Craig Watson
               Michael Garris
      DATE:    01/17/2001
      UPDATED: 04/25/2005 by MDG

      Contains routines responsible for determining the scan
      resolution of a WSQ compressed image by attempting to
      locate and parse a NISTCOM comment in the datastream.

      ROUTINES:
#cat: read_ppi_wsq - Given a WSQ compressed data stream, attempts to
#cat:                read a NISTCOM comment from an open file and
#cat:                if possible return the pixel scan resulution
#cat:                (PPI value) stored therein.
#cat: getc_ppi_wsq - Given a WSQ compressed data stream, attempts to
#cat:                read a NISTCOM comment from a memory buffer and
#cat:                if possible return the pixel scan resulution
#cat:                (PPI value) stored therein.

***********************************************************************/

#include <stdio.h>
#include <wsq.h>

/************************************************************************/
int read_ppi_wsq(int *oppi, FILE *infp)
{
   int ret;
   long savepos;
   int ppi;
   char *value;
   NISTCOM *nistcom;

   /* Save current position in filestream ... */
   if((savepos = ftell(infp)) < 0){
      fprintf(stderr, "ERROR : read_ppi_wsq : ");
      fprintf(stderr, "ftell : couldn't determine current position\n");
      return(-2);
   }
   /* Set file pointer to beginning of filestream ... */
   if(fseek(infp, 0L, SEEK_SET) < 0){
      fprintf(stderr, "ERROR : read_ppi_wsq : ");
      fprintf(stderr, "fseek : couldn't set pointer to start of file\n");
      return(-3);
   }

   /* Get ppi from NISTCOM, if one exists ... */
   if((ret = read_nistcom_wsq(&nistcom, infp))){
      /* Reset file pointer to original position in filestream ... */
      if(fseek(infp, savepos, SEEK_SET) < 0){
         fprintf(stderr, "ERROR : read_ppi_wsq : ");
         fprintf(stderr, "fseek : couldn't reset file pointer\n");
         return(-4);
      }
      return(ret);
   }
   if(nistcom != (NISTCOM *)NULL){
      if((ret = extractfet_ret(&value, NCM_PPI, nistcom))){
         freefet(nistcom);
         /* Reset file pointer to original position in filestream ... */
         if(fseek(infp, savepos, SEEK_SET) < 0){
            fprintf(stderr, "ERROR : read_ppi_wsq : ");
            fprintf(stderr, "fseek : couldn't reset file pointer\n");
            return(-5);
         }
         return(ret);
      }
      if(value != (char *)NULL){
         ppi = atoi(value);
         free(value);
      }
      /* Otherwise, PPI not in NISTCOM, so ppi = -1. */
      else
         ppi = -1;
      freefet(nistcom);
   }
   /* Otherwise, NISTCOM does NOT exist, so ppi = -1. */
   else
      ppi = -1;

   /* Reset file pointer to original position in filestream ... */
   if(fseek(infp, savepos, SEEK_SET) < 0){
      fprintf(stderr, "ERROR : read_ppi_wsq : ");
      fprintf(stderr, "fseek : couldn't reset file pointer\n");
      return(-6);
   }

   *oppi = ppi;

   return(0);
}

/************************************************************************/
int getc_ppi_wsq(int *oppi, unsigned char *idata, const int ilen)
{
   int ret;
   int ppi;
   char *value;
   NISTCOM *nistcom;

   /* Get ppi from NISTCOM, if one exists ... */
   if((ret = getc_nistcom_wsq(&nistcom, idata, ilen)))
      return(ret);
   if(nistcom != (NISTCOM *)NULL){
      if((ret = extractfet_ret(&value, NCM_PPI, nistcom))){
         freefet(nistcom);
         return(ret);
      }
      if(value != (char *)NULL){
         ppi = atoi(value);
         free(value);
      }
      /* Otherwise, PPI not in NISTCOM, so ppi = -1. */
      else
         ppi = -1;
      freefet(nistcom);
   }
   /* Otherwise, NISTCOM does NOT exist, so ppi = -1. */
   else
      ppi = -1;

   *oppi = ppi;

   return(0);
}
