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
      LIBRARY: UTIL - General Purpose Utility Routines

      FILE:    TICKS.C
      AUTHOR:  JAMES BLUE
      DATE:    11/13/1990

      Contains routines responsible for accumulating the number of clock
      cycles (ticks) used by a process.

      ROUTINES:
#cat: ticks - returns the number of clock cycles (ticks) used by a process.
#cat:
#cat: ticksPerSec - returns the system definition for HZ, where 1/HZ seconds
#cat:               is a tick (HZ = 60).

***********************************************************************/
#include <sys/types.h>
#include <sys/times.h>
#include <params.h>
#include <time.h>
#include <unistd.h>

/* ticks 13-Nov-90 15:41
 *		Get number of ticks used by process.
 */
unsigned long
ticks(void)
{
	struct tms buff;
	
	times(&buff);
	return buff.tms_utime;
}


/* ticksPerSec 13-Nov-90 15:41
 *		Get number of ticks per second reported by times().
 */
int
ticksPerSec(void)
{
	return (int)sysconf(_SC_CLK_TCK);
}

