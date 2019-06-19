@echo off
@indent %1 -o indent.$$$
@del %1
@ren indent.$$$ %1
