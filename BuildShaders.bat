@echo off

SET shadersBinDir=..\Data\Shaders
SET shadersSrcDir=Shaders

rem cd to this file's directory
cd %~dp0

CompileShaders.bat %shadersBinDir% %shadersSrcDir% 2>&1 3>&1
