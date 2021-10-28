[![Build status](https://ci.appveyor.com/api/projects/status/l9hq9qglvn6q5wdg/branch/main?svg=true)](https://ci.appveyor.com/project/Alia5/glossi/branch/main) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Github All Releases](https://img.shields.io/github/downloads/Alia5/GloSC/total.svg)]() [![Discord](https://img.shields.io/discord/368823110817808384.svg)](https://discord.gg/T9b4D5y)

TODO: Logo
<!-- ![GloSC logo](https://github.com/Alia5/GloSC/blob/master/GloSC_Icon_small.png?raw=true "GloSC logo") -->

# ATTENTION: GloSC is currently being rewritten, and renamed to GlosSI ([Glo]bal ([s]ystemwide) [S]team [I]nput)

No ETA when it's done  
No support until then.

**Looking for contributors!**  
As the past has shown, I have way to less time on hand too maintain such a project.  
Reach out via Discord/E-Mail (But get to the point right away, please, I get way too much spam)

# GlosSI

GlosSI or [Glo]bal ([s]ystemwide) [S]team [I]nput is a tool that allows one to use Steam-Input controller rebinding at a system-level alongside a system wide (borderless window) Steam overlay  
All complete with **per application bindings and working rumble emulation.**  
GlosSI can, but isn't required to, launch any of your favorite games or applications and directly add them to Steam, be it Win32 or UWP!  
It is **the tool** to enjoy any game that has trouble with Steam and/or *add extra functionality* to your Steam-Input needs  

*UWP*, *Reshade / SweetFX*, *Origin*, *Uplay*, *Emulators* and *more* with **no hassle**

---

**How does it work? / What does it do?**

GlosSI creates and adds a (or multiple) non-Steam shortcuts to Steam. When one of those is launched, a transparent, borderless window appears in which you can use the Steam-overlay. You also get access to touch- and radial-menus and other functionality normally only present in Games

In addition to that, Gamecontroller-inputs are redirected to the whole operating system, so that they will work with any game or application

This brings full Steam-Input functionality to the desktop and any other application Steam-Input might not have worked before

---

**Cleaning up misconceptions**

GlosSI doesn't hook into any of your games, launched programs or any system-component, except into Steam itself to keep the controller from switching to the desktop-config (if not disabled)

Games do not need to be launched using GlosSI.  

If the "Start Application" option does not work, launch any GlosSI-Shortcut from Steam, followed by a game or application.

---

## FAQ / Troubleshooting

* **Q: The overlay isn't showing up!**

  A: The overlay only works for windowed or borderless windowed mode applications.  
     This is even true for UWP-Games! While it's true that UWP doesn't run exclusive fullscreen, some Windows bullshit prevents unsigned apps from drawing over them.  
     Try running your games in borderless window mode

  A: Enable Xbox360 controller rebinding in Steam! It's in the "Controller" section in the Steam Settings.

* **Q: GlosSI overlay causes the screen to be black**

  A: If running on a Laptop with NVidia-GPU, open up NVidia control panel (right click your Desktop and select it). Manage the 3D-settings and have it auto select your graphics processor.

  Alternatively there is the option to run GlosSI in windowed mode.
  Simply open the GlosSI overlay by toggling Steams overlay twice; You cna change it from there.
  This will, unfortunately, also somewhat prevent you from using touch- and radial-menus.

  Instructions for AMD GPUs should be similar, in case any issues occur.        

---

Join the GlosSI discord here: https://discord.gg/T9b4D5y

---

**GlosSI consists of:**
- The "GlosSITarget" which does most of the magic - Showing the overlay to the user as well as talking to the ViGEm-driver for system wide Controller emulation
- A config application ("GlosSIConfig") handling shortcut ("GlosSITarget") creation and their addition to Steam.

---

Like my stuff? Hit me up [on twitter](https://twitter.com/Flatspotpics) or consider donating to my [PayPal](https://www.paypal.me/Flatspotpics)

GloSC got mentioned from Valve in the [Steam client beta change log on the 9. of January](https://twitter.com/flatspotpics/status/818697837055770624)

For Building / Manual installation refer to the **TODO**

GlosSI is not affiliated with Valve, Steam, or any of their partners.

---

GlosSI is built using [Qt 6.2](https://www.qt.io/) and a fork of [SFML](http://www.sfml-dev.org/) for drawing the overlay

The system wide Xbox-Controller works via [ViGEm](https://vigem.org/projects/ViGEm/)
Device Hiding via [HidHide](https://vigem.org/projects/HidHide/)

## License

```license
Copyright 2017-2018 Peter Repukat - FlatspotSoftware

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
