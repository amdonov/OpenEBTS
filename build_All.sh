#!/bin/sh

# build Debug and Release flavours of NBISWSQ library
cd ./libs/NBISWSQ/Debug
make all
cd ./Release
make all
cd ./../../..

# build Debug Flavors
cd ./OpenEBTS/Debug
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/Debug
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/Debug
make all
cd ./../../..

# build DebugA Flavors
cd ./OpenEBTS/DebugA
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/DebugA
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/DebugA
make all
cd ./../../..

# build Release Flavors
cd ./OpenEBTS/Release
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/Release
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/Release
make all
cd ./../../..

# build ReleaseA Flavors
cd ./OpenEBTS/ReleaseA
make all
cd ./../../OpenEBTSSamples/OpenEBTSSample1/ReleaseA
make all
cd ./../../../OpenEBTSSamples/OpenEBTSSample2/ReleaseA
make all
cd ./../../..

