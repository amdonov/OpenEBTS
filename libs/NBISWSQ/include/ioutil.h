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


#ifndef _IOUTIL_H
#define _IOUTIL_H

#ifndef True
#define True	1
#define False	0
#endif

#define MaxLineLength	512
#define EOL	EOF

/* fileexst.c */
extern int file_exists(char *);
/* filehead.c */
extern void filehead(char *);
/* fileroot.c */
extern void fileroot(char *);
/* filesize.c */
extern int filesize(char *);
/* filetail.c */
extern void filetail(char *);
/* findfile.c */
extern int find_file(char *, char *);
/* newext.c */
extern void newext(char *, const int, char *);
extern int newext_ret(char *, int, char *);
extern void newextlong(char **, char *);
/* readutil.c */
extern int read_strstr_file(char *, char ***, char ***, int *, const int);
extern int read_fltflt_file(char *, float **, float **, int *, const int);

#endif /* !_IOUTIL_H */
