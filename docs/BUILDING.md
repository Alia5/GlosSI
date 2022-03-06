---
---

# Building GlosSI

## Windows

Requirements:

- Visual Studio 2022 (Community edition is fine)
- Qt 6.2.X (GlosSIConfig only)
- Qt Visual Studio addin (GlosSIConfig only)

```powershell
.\prebuild.ps1

# Open GlosSI.sln and hit build!
Invoke-Item GlosSI.sln
```

In addition to the above, you will need to install the required drivers:  
`ViGEmBusSetup_x64.msi` and `HidHideMSI.msi`  
Both of which can be downloed from [ViGEm's website](https://vigem.org/Downloads/) or by use of the `download_release_deps.ps1` script.

## Linux

Linux support is currently not really implemented.  
That said, very limited linux support is planned for the future.  
No Guarantees that the build works out!

Building should be as easy as:

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
