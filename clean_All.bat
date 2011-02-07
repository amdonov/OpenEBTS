REM ***
REM *** After a build of OpenEBTS.sln, this batch file should remove all the files
REM *** created by that build, and leave everything as it was before the build.
REM ***

del /S *.obj
del /S /A:H *.suo
del /S *.embed.manifest
del /S *.embed.manifest.res
del /S *.intermediate.manifest
del /S *.exp
del /S *.ncb
del /S *.ilk
del /S *.res
del /S *.idb
del /S *.dep
del /S *.pch
del /S *.aps
del /S *.vcproj.*.user
del /S BuildLog.htm
del /S OpenEBTS.dll
del /S OpenEBTS.lib
del /S OpenEBTSa.dll
del /S OpenEBTSa.lib
del /S OpenEBTSSample*.exe
del /S OpenEBTSViewer*.exe
// Leave the NBISWSQ pdb files alone so we can link against them for debug info but
// ditch all the others, which means going into each of the 3 folders explicitly.
cd OpenEBTS
del /S *.pdb
cd ..
cd OpenEBTSSamples
del /S *.pdb
cd ..
cd OpenEBTSViewer
del /S *.pdb
cd ..
