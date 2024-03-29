@echo off

set shadersBinDir=%1
set shadersSrcDir=%2

rem Compile all shaders
for %%f in (%shadersSrcDir%\*_fs.hlsl) do (
    echo Compiling %%~nf_fs.hlsl
    dxc -spirv -O3 -T ps_6_0 -E main %%f -Fo %shadersBinDir%\%%~nf.spv -D FS
)

for %%f in (%shadersSrcDir%\*_vs.hlsl) do (
    echo Compiling %%~nf_vs.hlsl
    dxc -spirv -O3 -T vs_6_0 -E main %%f -Fo %shadersBinDir%\%%~nf.spv -D VS
)
