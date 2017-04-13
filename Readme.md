[![Build status](https://ci.appveyor.com/api/projects/status/ph7g8xcct9hab6fp?svg=true)](https://ci.appveyor.com/project/Alia5/glosc) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

![GloSC logo](https://github.com/Alia5/GloSC/blob/master/GloSC_Icon_small.png?raw=true "GloSC logo")

# GloSC

GloSC or Global SteamController is a small set of tools that allows you to use your SteamController as a system wide XInput-controller alongside a system wide (borderless window) Steam overlay

All complete with **per application bindings and working rumble emulation.**

GloSC can also launch any of your favorite Games and directly add them to Steam, be it Win32 or UWP!

It is **the tool** to enjoy any Game that has trouble with Steam and/or *add extra functionality* to your SteamController

```
```

## FAQ / Troubleshooting

* **Q: The overlay isn't showing up!**

  A: The overlay only works for windowed or borderless windowed mode applications. Nothing much I can do here.
     This is even true for UWP-Games! While it's true that UWP doesn't run exclusive fullscreen, they don't run, strictly speaking, classical borderless window either
     
     Try running your games as borderless window
     
* **Q: Audio turns off when using this**

  A: ViGEm emulates a full X360-controller incl. peripherals
  
  Go to your windows sound mixer and reset you default playback device - It should be fixed, even for the next shortcut launches

* **Q: Multiple different controllers are not working**

  A: Will probably be fixed in the long run, for now we have to wait for a new ViGEm release.

* **Q: Hitting the Steam Button + RT to take a screenshot doesn't work**

  A: Another limitation that most likely won't be fixed, even if Steam would take a screenshot, you'd most likely get just a blank image

* **Q: UWP Games don't launch in foreground**

  A: For some reason trying to launch them in foreground stops them from launching entirely
  
     For now use alt+tab via Steam-chords
     
     This is currently being investigated
```
```

It is built using [Qt 5.7](https://www.qt.io/) and uses [SFML](http://www.sfml-dev.org/) for drawing the overlay

The system wide Xbox-Controller works via [ViGEm](https://github.com/nefarius/ViGEm)

GloSC consists of:
 
 * A main application ("GloSC") allowing you to create and configure new shortcuts or "SteamTargets" and automatically add them to Steam
 * The "GloSC-GameLauncher" which can launch Win32 and UWP games and mainly exists to work around dual overlays
 * The "SteamTarget" which does most of the magic - Showing the overlay to the user as well as talking to the ViGEm-driver for system wide Controller emulation

 
Like my stuff? Hit me up [on twitter](https://twitter.com/Flatspotpics) or consider donating to my [PayPal](https://www.paypal.me/Flatspotpics)

GloSC got mentioned from Valve in the [Steam client beta change log on the 9. of January](https://twitter.com/flatspotpics/status/818697837055770624)

You can learn more about how GloSC works [here](https://behind.flatspot.pictures/third-party-steam-controller-software-part2-my-take-on-it/)

For Building / Manual installation refer to the [build guide](https://github.com/Alia5/GloSC/blob/master/Build_Install.md)


## License

```
Copyright 2016 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
