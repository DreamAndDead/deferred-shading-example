
cd C:

pushd C:\msys64\home\favor\project\deferred-shading-example


set fx_list=(PointLight GBuffer)


for %%i in %fx_list% do "%DXSDK_DIR%/Utilities/bin/x86/fxc.exe" /Tfx_2_0 /Gfp /nologo /Fo %%i.fxo %%i.hlsl

popd