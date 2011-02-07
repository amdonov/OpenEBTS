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
      LIBRARY: IOUTIL - INPUT/OUTPUT Utilities

      FILE:    FILESIZE.C
      AUTHOR:  Michael Garris
      DATE:    12/05/2001

      Contains routines responsible for determining the size of
      a file in bytes given its path name.

      ROUTINES:
#cat: filesize - opens the specified pathname and determines the size of
#cat:            of the file in bytes.

***********************************************************************/

#include <stdio.h>
#ifndef WIN32			//DJD
#include <unistd.h>		//DJD
#endif					//DJD

/**********************************************************************/
int filesize(char *ifile)
{
   FILE *fp;
   int ret;

   /* Open file. */
   if((fp = fopen(ifile, "rb")) == (FILE *)NULL){
      fprintf(stderr, "ERROR : filesize : file %s could not be opened\n",
              ifile);
      return(-2);
   }

   /* Move file pointer to end of file. */
   if(fseek(fp, 0, SEEK_END)){
      fprintf(stderr, "ERROR : filesize : seeking to EOF of file %s failed\n",
              ifile);
      return(-3);
   }

   /* Get byte offest to end of file. */
   if((ret = ftell(fp)) < 0){
      fprintf(stderr, "ERROR : filesize : ftell at EOF of file %s failed\n",
              ifile);
      return(-4);
   }

   /* Close file. */
   fclose(fp);

   /* Return size of file in bytes. */
   return(ret);
}
