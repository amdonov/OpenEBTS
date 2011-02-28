
To obtain the 4 binaries, libcurl.lib, libcurld.lib,  x64/libcurl.lib and x64/libcurld.lib,
the following steps were performed:


1) The sources for libcurl 7.21.4 were downloaded from http://curl.haxx.se. Starting with the
sources was necessary because there was no officially downloadable 64-bit binary for Windows.

2) After extraction of the compressed package, the file vc6curl.dsw was converted into an sln
with Visual Studio 2008 SP1.

3) The solution platform x64 was added in the Configuration Manager dialog.

4) - Lib Release Win32 was built, producing a Win32 static library.
   - Lib Release x64 was built, producing a Win64 static library.
   - Lib Debug Win32 was built, producing a Win32 debug static library.
   - Lib Debug x64 was built, producing a Win64 debug static library.
