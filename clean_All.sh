#!/bin/sh

# clean Debug and Release flavours of NBISWSQ library
cd ./libs/NBISWSQ/Debug
make clean
cd ./../Release
make clean
cd ./../../..

# clean Debug flavors
cd ./OpenEBTS/Debug
make clean
cd ./../../OpenEBTSSamples/OpenEBTSSample1/Debug
make clean
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/Debug
make clean
cd ./../../..

# clean DebugA flavors
cd ./OpenEBTS/DebugA
make clean
cd ./../../OpenEBTSSamples/OpenEBTSSample1/DebugA
make clean
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/DebugA
make clean
cd ./../../..

# clean Release flavors
cd ./OpenEBTS/Release
make clean
cd ./../../OpenEBTSSamples/OpenEBTSSample1/Release
make clean
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/Release
make clean
cd ./../../..

# clean ReleaseA flavors
cd ./OpenEBTS/ReleaseA
make clean
cd ./../../OpenEBTSSamples/OpenEBTSSample1/ReleaseA
make clean
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/ReleaseA
make clean
cd ./../../..

