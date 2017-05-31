SET QTDIR="C:\Qt\5.9\msvc2017_64"
SET APPVEYOR_BUILD_FOLDER="D:\Alia5\Documents\Visual Studio Projects\GloSC"

"C:\Program Files\7-Zip\7z.exe" a GloSC_x64.zip TargetConfig.ini License.txt qt-license.txt Readme.md Build_Install.md %APPVEYOR_BUILD_FOLDER%\build\x64\Release\*.exe %APPVEYOR_BUILD_FOLDER%\build\Win32Only\Release\*.exe %APPVEYOR_BUILD_FOLDER%\build\Win32Only\Release\*.dll %APPVEYOR_BUILD_FOLDER%\dependencies\SFML-2.4.2-x64\bin\sfml-system-2.dll %APPVEYOR_BUILD_FOLDER%\dependencies\SFML-2.4.2-x64\bin\sfml-window-2.dll %APPVEYOR_BUILD_FOLDER%\dependencies\SFML-2.4.2-x64\bin\sfml-graphics-2.dll %APPVEYOR_BUILD_FOLDER%\dependencies\ViGEmUM\x64\ViGEmUM.dll redist\* %QTDIR%\bin\Qt5Core.dll %QTDIR%\bin\Qt5Gui.dll %QTDIR%\bin\Qt5Widgets.dll %QTDIR%\plugins\platforms\qwindows.dll

"C:\Program Files\7-Zip\7z.exe" rn GloSC_x64.zip qwindows.dll platforms\qwindows.dll