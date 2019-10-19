SET QTDIR="D:\Qt\5.13.1\msvc2017"
SET APPVEYOR_BUILD_FOLDER="D:\Alia5\Documents\Visual_Studio_Projects\GloSC"

"C:\Program Files\7-Zip\7z.exe" a GloSC.zip TargetConfig.ini License.txt dependencies\minhook\MH_LICENSE.txt qt-license.txt Readme.md Build_Install.md %APPVEYOR_BUILD_FOLDER%\build\Win32\Release\*.exe %APPVEYOR_BUILD_FOLDER%\build\Win32\Release\*.dll %APPVEYOR_BUILD_FOLDER%\dependencies\SFML-2.4.2-x86\bin\sfml-system-2.dll %APPVEYOR_BUILD_FOLDER%\dependencies\SFML-2.4.2-x86\bin\sfml-window-2.dll %APPVEYOR_BUILD_FOLDER%\dependencies\SFML-2.4.2-x86\bin\sfml-graphics-2.dll redist\* %QTDIR%\bin\Qt5Core.dll %QTDIR%\bin\Qt5Gui.dll %QTDIR%\bin\Qt5Network.dll %QTDIR%\bin\Qt5Widgets.dll %QTDIR%\plugins\platforms\qwindows.dll

"C:\Program Files\7-Zip\7z.exe" rn GloSC.zip qwindows.dll platforms/qwindows.dll
"C:\Program Files\7-Zip\7z.exe" rn GloSC.zip redist/ViGEmClient.dll ViGEmClient.dll

"C:\Program Files\7-Zip\7z.exe" rn GloSC.zip redist/libeay32.dll libeay32.dll
"C:\Program Files\7-Zip\7z.exe" rn GloSC.zip redist/ssleay32.dll ssleay32.dll