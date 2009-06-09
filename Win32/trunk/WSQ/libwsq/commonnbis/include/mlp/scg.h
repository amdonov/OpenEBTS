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

#ifndef _SCG_H
#define _SCG_H

#define XLSTART 0.01 /* Starting value for xl. */
#define NF 3         /* Don't quit until NF * nfreq iters or... */
#define NITER 40     /* ...until NITER iters, whichever is larger... */
#define NBOLTZ 100   /* ...until NBOLTZ iters, if doing Boltzmann. */
#define NNOT 3       /* Quit if not improving NNOT times in row. */
#define NRESTART 100000 /* Restart after NRESTART iterations. */

#endif /* !_SCG_H */
