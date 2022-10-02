# Building GlosSI

## Windows

Requirements:

- git
- Visual Studio 2022 (Community edition is fine)
- [Qt 6.X](https://www.qt.io/download-qt-installer) (GlosSIConfig only)
- [Qt Visual Studio addin](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2022) (GlosSIConfig only)

In a "Developer Powershell for VS 2022" run:

```powershell
git submodule init
git submodule update --recursive

.\prebuild.ps1

# Open GlosSI.sln and hit build!
Invoke-Item GlosSI.sln
```

(Note: It can be the case that the first build fails as there are versioning files created when building. Just hit build again and you should be good to go!)

In addition to the above, you will need to install the required drivers:  
`ViGEmBusSetup_x64.exe` and `HidHideMSI.exe`  
Both of these can be downloaded from [ViGEm's website](https://vigem.org/Downloads/) or by use of the `download_release_deps.ps1` script.

---

## Linux

Linux support is currently not really implemented.  
No guarantees that the build will even work. (It's probably broken)  
That said, very limited linux support is planned for the future.  
No Guarantees that the build works out!
**GlosSITarget:**

```shell
git submodule init
git submodule update --recursive

# build custom fork of SFML
# do not use SFML you might've already installed
./buildSFML.sh 
cd GlosSITarget
cmake -S . -B build
cmake --build build
```

**GlosSIConfig:**

TODO
