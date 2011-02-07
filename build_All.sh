#!/bin/sh

#Build Debug Flavors
cd ./OpenEBTS/Debug
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/Debug
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/Debug
make all
cd ./../../..

#Build DebugA Flavors
cd ./OpenEBTS/DebugA
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/DebugA
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/DebugA
make all
cd ./../../..

#Build Release Flavors
cd ./OpenEBTS/Release
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/Release
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/Release
make all
cd ./../../..

#Build ReleaseA Flavors
cd ./OpenEBTS/ReleaseA
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/ReleaseA
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/ReleaseA
make all
cd ./../../..

