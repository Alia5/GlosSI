[![Build status](https://ci.appveyor.com/api/projects/status/ph7g8xcct9hab6fp?svg=true)](https://ci.appveyor.com/project/Alia5/glosc) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Github All Releases](https://img.shields.io/github/downloads/Alia5/GloSC/total.svg)]()

![GloSC logo](https://github.com/Alia5/GloSC/blob/master/GloSC_Icon_small.png?raw=true "GloSC logo")

# GloSC

Global SteamController (GloSC) is a small set of tools that allows the SteamController to be used as a system wide XInput-controller alongside the Steam overlay.

This tool allows the enjoyment of any game that has trouble with Steam while *adding extra functionality* to the SteamController, such as **per application bindings and a working rumble emulation.**

GloSC can launch any game and directly add them to Steam, be it Win32 or UWP!

Play *UWP*, *Origin* and *Uplay*-Games, and use *Reshade / SweetFX* with **no hassle**!

---

**How does it work? / What does it do?**

GloSC creates and adds non-Steam shortcuts to Steam. When a shortcut is launched, a transparent, borderless window allowing the use of the Steam-overlay appears. GloSC also allows access to touch- and radial-menus as well as other functionalities normally only present in games.

XInput-inputs are redirected to the whole operating system, in order to function with any game or application, enabling full SteamController functionality to any application that previously could not be used with the controller.

---

**Cleaning up misconceptions**

GloSC doesn't hook into any games, launched programs or system-component. The user can choose to have GloSC hook into Steam to keep the controller from switching to the desktop-config.

Games do not need to be launched using GloSC.  
If the "Start Application" option does not work, launch any GloSC-Shortcut from Steam, followed by a game or application.


```
```

## FAQ / Troubleshooting

* **Q: The overlay isn't showing up!**

  A: The overlay only works for windowed or borderless windowed mode applications.
     Even though UWP-Games do not run exclusive fullscreen, Windows prevents unsigned apps from drawing over UWP-Games so the overlay will not work.

     Try running the game in borderless windowed mode.

* **Q: GloSC always creates four controllers**

  A: Enable Xbox360 controller rebinding in Steam! It's in the "Controller" section in the Steam Settings.

* **Q: Some games (Gears of War 4, Forza Horizons 3) have odd double inputs with the bumper buttons and sticks**

  A: Enable Xbox360 controller rebinding in Steam! It's in the "Controller" section in the Steam Settings.

* **Q: Hitting the Steam Button + RT to take a screenshot doesn't work**

  A: This is another limitation that most likely won't be fixed. Even if Steam can take a screenshot, only a blank image would be obtained.

* **Q: GloSC overlay causes the screen to be black**

  A: If GloSC is being run on a mobile, open up Nvidia control panel (right click your Desktop and select it). Manage the 3D-settings and have it auto select your graphics processor. Otherwise, add an exclusive fix for Steam on the program settings tab.        

```
```

Join the GloSC discord here: https://discord.gg/T9b4D5y

---

**GloSC consists of:**

 * A main application ("GloSC") allowing the creation and configuration of new shortcuts ("SteamTargets") their automatic addition to Steam
 * The "SteamTarget" which does most of the magic - Showing the overlay to the user as well as talking to the ViGEm-driver for system wide Controller emulation

---

Like my stuff? Hit me up [on twitter](https://twitter.com/Flatspotpics) or consider donating to my [PayPal](https://www.paypal.me/Flatspotpics)

GloSC got mentioned from Valve in the [Steam client beta change log on the 9. of January](https://twitter.com/flatspotpics/status/818697837055770624)

You can learn more about how GloSC works [here](https://behind.flatspot.pictures/third-party-steam-controller-software-part2-my-take-on-it/)

For Building / Manual installation refer to the [build guide](https://github.com/Alia5/GloSC/blob/master/Build_Install.md)

GloSC is not affiliated with Valve, Steam, or any of their partners.

---

GloSC is built using [Qt 5.X](https://www.qt.io/) and [SFML](http://www.sfml-dev.org/) for drawing the overlay

The system wide Xbox-Controller works via [ViGEm](https://github.com/nefarius/ViGEm)


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
