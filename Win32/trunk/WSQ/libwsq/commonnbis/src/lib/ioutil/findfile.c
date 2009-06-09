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

      FILE:    FINDFILE.C
      AUTHOR:  Michael Garris
      DATE:    11/27/1989
      UPDATED: 03/15/2005 by MDG

      Contains routines responsible for locating a file within a
      specified directory path.

      ROUTINES:
#cat: find_file - takes a directory path and a filename and determines if
#cat:             the file exists.

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <dirent.h>

#define TRUE 1
#define FALSE 0

/**********************************************************************/
int find_file(char *path, char *file)
{
    int flag,len;
    DIR *dirp;
    struct dirent *dp;

    flag = 0;
    if(strcmp(path,"") != 0){
        len = strlen(file);
        dirp = opendir(path);
        for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
#ifdef __STDC__
           if((strlen(dp->d_name) == len) && !strcmp(dp->d_name,file)){
#else
           if(dp->d_namlen == len && !strcmp(dp->d_name,file)){
#endif
              flag = 1;
           }
        }
        closedir(dirp);
    }
    
    if (flag == 1) 
        return(TRUE);
    else
        return(FALSE);
}
