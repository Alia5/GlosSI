cd deps/ViGEmClient

git apply ../../ViGEm_BuildConfig.patch

$env:_CL_="/MDd"
msbuild.exe ViGEmCLient.sln /t:Build /p:Configuration=Debug_LIB /p:Platform=x64
$env:_CL_="/MD"
msbuild.exe ViGEmCLient.sln /t:Build /p:Configuration=Release_LIB /p:Platform=x64

cd ../..