This project is based on the NBIS source from http://www.nist.gov/itl/iad/ig/nbis.cfm,
Release 3.3.1.

It was created by compiling the minimal set of files necessary to support wsq_encode_mem
and wsq_decode_mem (wsq/encoder.c, wsq/decoder.c). These files are:

.\fet\allocfet.c
.\fet\delfet.c
.\fet\extrfet.c
.\fet\freefet.c
.\fet\lkupfet.c
.\fet\nistcom.c
.\fet\strfet.c
.\fet\updatfet.c
.\ioutil\dataio.c
.\ioutil\filesize.c
.\jpegl\_huff.c
.\jpegl\_tableio.c
.\jpegl\huftable.c
.\util\computil.c
.\util\fatalerr.c
.\util\syserr.c
.\wsq\decoder.c
.\wsq\encoder.c
.\wsq\globals.c
.\wsq\huff.c
.\wsq\ppi.c
.\wsq\tableio.c
.\wsq\tree.c
.\wsq\util.c

Note that the files preceeded by an underscore were so renamed to avoid conflicts
within Visual Studio 2008. All necessary include files, once again the minimal
required set, were all placed in the folder 'include'.

These source must be compiled with the __NBISLE__ flag on both Windows and Linux
since they are both little endian operating systems.
