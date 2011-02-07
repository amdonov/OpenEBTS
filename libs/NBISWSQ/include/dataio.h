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


#ifndef _DATA_IO_H
#define _DATA_IO_H

/* dataio.c */
extern int read_byte(unsigned char *, FILE *);
extern int getc_byte(unsigned char *, unsigned char **, unsigned char *);
extern int getc_bytes(unsigned char **, const int, unsigned char **, 
                 unsigned char *);
extern int write_byte(const unsigned char, FILE *);
extern int putc_byte(const unsigned char, unsigned char *, const int, int *);
extern int putc_bytes(unsigned char *, const int, unsigned char *,
                 const int, int *);
extern int read_ushort(unsigned short *, FILE *);
extern int getc_ushort(unsigned short *, unsigned char **, unsigned char *);
extern int write_ushort(unsigned short, FILE *);
extern int putc_ushort(unsigned short, unsigned char *, const int, int *);
extern int read_uint(unsigned int *, FILE *);
extern int getc_uint(unsigned int *, unsigned char **, unsigned char *);
extern int write_uint(unsigned int, FILE *);
extern int putc_uint(unsigned int, unsigned char *, const int, int *);
extern void write_bits(unsigned char **, const unsigned short, const short,
                 int *, unsigned char *, int *);
extern void flush_bits(unsigned char **, int *, unsigned char *, int *);
extern int read_ascii_file(char *, char **);

#endif /* !_DATA_IO_H */
