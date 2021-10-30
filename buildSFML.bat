cd deps/SFML

if  "%APPVEYOR%"=="" (
cmake.exe -S . -B out/Debug -DCMAKE_BUILD_TYPE=Debug
cmake.exe --build out/Debug --config Debug
)

cmake.exe -S . -B out/Release -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake.exe --build out/Release --config RelWithDebInfo

cd ../..