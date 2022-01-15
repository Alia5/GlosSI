cd deps/ViGEmClient

git apply ../../ViGEm_BuildConfig.patch

msbuild.exe ViGEmCLient.sln /t:Build /p:Configuration=Debug_LIB /p:Platform=x64
msbuild.exe ViGEmCLient.sln /t:Build /p:Configuration=Release_LIB /p:Platform=x64

cd ../..