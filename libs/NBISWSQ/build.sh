#!/bin/sh

cd ./Debug
make all
cd ./../Release
make all
cd ./..

