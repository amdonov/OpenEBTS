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
      LIBRARY: FING - NIST Fingerprint Systems Utilities

      FILE:           BUBBLE.C
      ALGORITHM:      Michael Garris
      DATE:           09/20/2004

***********************************************************************

      ROUTINES:
#cat: bubble_sort_int - sorts a list of integers into decreasing order with a
#cat:                   bubble sort. It is destructive to the integer list
#cat:                   passed to it.

***********************************************************************/

#include<swap.h>

/***************************************************************************/
void bubble_sort_int(int *pts, const int n)
{
   int done = 0;
   int i, p;

   p = n;
   while(!done){
    done = 1;
    for (i=1;i<p;i++){
      if(pts[i-1] < pts[i]){
         swap_int(pts[i-1], pts[i]);
         done = 0;
      }
    }
    p--;
  }     
}
