git submodule init
git submodule update --recursive

cmd.exe /c buildSFML.bat
cmd.exe /c buildViGEmClient.bat

cd deps/traypp

git apply ../../traypp_unicode.patch

cd ../../