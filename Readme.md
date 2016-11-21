# GloSC

GlosSC or Global SteamController is a small set of tools that allows you to use your SteamController as a systemwide XInput-controller alongside a systemwide (borderless window) Steam overlay
All complete with **per application bindings and working rumble emulation.**

GloSC can also launch any of your favorite Games and directly add them to Steam, be it Win32 or UWP!

It is **the tool** to enjoy any Game that has trouble with Steam and add extra functionality to your SteamController

```
```

## FAQ / Troubleshooting

* **Q: The overlay isn't showing up!**
  A: The overlay only works for windowed or borderless windowed mode applications. Nothing much I can do here.
     This is even true for UWP-Games! While it's true that UWP doesn't run exclusive fullscreen, they don't run, strictly speaking, classical borderless window either
     Try running your games as borderless window

* **Q: Hitting the Steam Button + RT to take a screenshot doesn't work**
  A: Another limitation that most likely won't be fixed, even if Steam would take a screenshot, you'd most likely get just a blank image

* **Q: UWP Games don't launch in foreground**
  A: For some reason trying to launch them in foreground stops them fron launching entirely
     For now use alt+tab via Steam-chords
     This is currently beeing investigated
  
* **Q: When using big picture the controller switches to desktop-config**
  A: Don't really know why this is happening
     For now launch your GloSC shortcuts via Steam desktop mode
     This is currently beeing investigated

```
```

It is built using [Qt 5.7](https://www.qt.io/) and uses [SFML](http://www.sfml-dev.org/) for drawing the overlay

The systemwide Xbox-Controller works via [ViGEm](https://github.com/nefarius/ViGEm)

GloSC consists of:
 
 * A main application ("GloSC") allowing you to create and configure new shortcuts or "SteamTargets" and automatically add them to Steam
 * The "GloSC-GameLauncher" which can launch Win32 and UWP games and mainly exists to work around dual overlays
 * The "SteamTarget" which does most of the magic - Showing the overlay to the user as well as talking to the ViGEm-driver for systemwide Controller emulation

 
Like my stuff? Consider donating to my [PayPal](https://www.paypal.me/Flatspotpics)

_Work in progress_


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
