git submodule init
git submodule update --recursive --force

.\buildSFML.ps1
.\buildViGEmClient.ps1

cd deps/traypp

git apply ../../traypp_unicode.patch

cd ../../