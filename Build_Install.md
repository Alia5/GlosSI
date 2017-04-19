# Building GloSC

**Build requirements:**
- M$ Visual Studio (2015)
- [Qt 5.7](http://info.qt.io/download-qt-for-application-development) (x86/x64)
- [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools)
- [ViGEm](https://github.com/nefarius/ViGEm)

ViGEms user mode library as well as [SFML](http://sfml-dev.org) are bundled in the repo.
For ViGEms installation refer to the Readme-files from the relevant sub-projects, or use the binary installer (if available).

--

Set the correct Qt version for "*GloSC*", "*GloSC_GameLauncher*" and "*SteamTarget*" depending on your build.

Rebuild the solution and you should be good to go.

# Installation

Copy all **files** out of the relevant build directories, as well as, "*TargetConfig.ini*" and the relevant SFML, ViGEmUM and QT .dll-files in a separate directory.

To make double sure unhooking Steam works properly, and allow games to launch without any issues, run "*GloSC_GameLauncher.exe*" **as Admin**
To do this at system start up you can create you can create this task using schtasks.exe
```
"schtasks.exe /create /f /tn "GloSC_GameLauncher" /tr "[PATH_TO]\GloSC_GameLauncher.exe" /sc onlogon /rl highest
```
Be careful that the right executable is running, as there can only ever be one Instance of "GloSC_GameLauncher" running.

Afterwards create your shortcuts with GloSC and you are ready to go.

### Notes:

The GloSC_GameLauncher must be running from that directory, as Admin, to make double sure Steam does get unhooked after closing a shortcut

