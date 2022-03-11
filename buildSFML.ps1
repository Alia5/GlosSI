cd deps/SFML

if ($env:APPVEYOR -like "") {
    cmake.exe -S . -B out/Debug -DCMAKE_BUILD_TYPE=Debug
    cmake.exe --build out/Debug --config Debug
}

$env:_CL_ = "/MDd"
cmake.exe -S . -B out/Release -DCMAKE_BUILD_TYPE=RelWithDebInfo
$env:_CL_ = "/MD"
cmake.exe --build out/Release --config RelWithDebInfo

cd ../..