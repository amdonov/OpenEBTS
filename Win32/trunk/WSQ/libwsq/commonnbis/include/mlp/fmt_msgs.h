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

#ifndef _FMT_MSGS_H
#define _FMT_MSGS_H

/* For use by strm_fmt() and lgl_tbl(), which format the warning and
error messages that may be written as the result of scanning a
specfile.  Columns are numbered starting at 0. */

#define MESSAGE_FIRSTCOL_FIRSTLINE   6 /* for first line of a msg */
#define MESSAGE_FIRSTCOL_LATERLINES  8 /* later lines indented */
#define MESSAGE_LASTCOL             70
#define MESSAGE_FIRSTCOL_TABLE      12 /* table indented even more */

#endif /* !_FMT_MSGS_H */
