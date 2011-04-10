REM ***
REM *** This batch file creates the JNI header file from the file bin/com/obi/OpenEBTS.class
REM *** and copies it to the Java folder.
REM ***

REM  *** creating OpenEBTS.h header file for JNI ***
echo *** creating OpenEBTS.h header file for JNI ***
javah -classpath bin -jni com.obi.OpenEBTS

REM  *** moving OpenEBTS to OpenEBTSJava folder, and renaming it OpenEBTSJNI.h to avoid conflicts ***
echo *** moving OpenEBTS to OpenEBTSJava folder, and renaming it OpenEBTSJNI.h to avoid conflicts ***
move com_obi_OpenEBTS.h ../OpenEBTSJNI.h

REM  *** removing other header files that are not used by the JNI ***
echo *** removing other header files that are not used by the JNI ***
erase *.h

pause
