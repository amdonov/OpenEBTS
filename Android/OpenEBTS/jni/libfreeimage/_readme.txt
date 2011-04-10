These files come directly from the folder 'Source' of the FreeImage 3.15.0 sourcepackage,
with the following exceptions:

- Plugin.h and plugin.cpp were edited to remove support for the RAW file format, which
was causing difficulties in compilation and is of little use to OpenEBTS.
- lfind.c was added as the Android r5b release doesn't seem to define lfind().
