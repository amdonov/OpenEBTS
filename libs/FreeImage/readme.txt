
The following 5 files

FreeImage.h
FreeImage.dll
FreeImage.lib
x64/FreeImage.dll
x64/FreeImage.lib

were obtained in the following fashion:

1) The FreeImage 3.15.0 source package was downloaded from http://freeimage.sourceforge.net/download.html.

2) The solution FreeImage.2008.sln was opened.

3) The Release|Win32 configuration was chosen and built. The newly created FreeImage.lib and FreeImage dll
were then taken from the 'Release' folder.

4) The Release|x64 configuration was chosen and built. The newly created FreeImage.lib and FreeImage dll
were then taken from the 'x64/Release' folder.

5) FreeImage.h was taken from the 'Source' folder.
