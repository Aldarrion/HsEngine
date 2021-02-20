@echo off

SET shadersBinDir=..\Data\Shaders
SET shadersSrcDir=Shaders

rem Compile all shaders
for %%f in (%shadersSrcDir%\*_fs.hlsl) do (
    dxc -spirv -O3 -T ps_6_0 -E main %%f -Fo %shadersBinDir%\%%~nf.spv -D FS
)

for %%f in (%shadersSrcDir%\*_vs.hlsl) do (
    dxc -spirv -O3 -T vs_6_0 -E main %%f -Fo %shadersBinDir%\%%~nf.spv -D VS
)
