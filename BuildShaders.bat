@echo off

SET shadersBinDir=..\Data\Shaders
SET shadersSrcDir=Shaders

call CompileShaders.bat %shadersBinDir% %shadersSrcDir%
