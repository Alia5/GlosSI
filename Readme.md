[![Build status](https://ci.appveyor.com/api/projects/status/l9hq9qglvn6q5wdg/branch/main?svg=true)](https://ci.appveyor.com/project/Alia5/glossi/branch/main) [![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) [![Discord](https://img.shields.io/discord/368823110817808384.svg)](https://discord.gg/T9b4D5y) ![version](https://img.shields.io/github/v/tag/alia5/glossi?label=version) [![WebsiteAndDownloads](https://img.shields.io/website?label=Website%20%26%20downloads&url=https%3A%2F%2Fglossi.flatspot.pictures)](https://glossi.flatspot.pictures)

<div style="display: flex; align-items: center">
<h1 align="center"><img align="center" src="https://github.com/Alia5/GlosSI/blob/main/GlosSI_Logo_512.png?raw=true" width="256" height="256" alt="GlosSI Logo" />
  GlosSI&nbsp;-&nbsp;Global&nbsp;(systemwide)&nbsp;Steam&nbsp;Input</h1>
</div>

GlosSI formerly knows as GloSC (Global Steam Controller), is a tool that allows one to use Steam-Input controller rebinding at a system-level alongside a system wide (borderless window) Steam overlay  

The primary use case of GlosSI is to use SteamInput (required for SteamController / SteamDeck-buttons) with incompatible Games such as Windows-Store titles or Emulators.  

GlosSI can, but isn't required to, launch any of your favorite games or applications and directly add them to Steam, be it Win32 or Windows Store (UWP)!  
It is **the tool** to enjoy any game that has trouble with Steam and/or *add extra functionality* to your Steam-Input needs  

---
```
```

# ViGEm End of Life

As you may or may not have already noticed ViGEm, a substantial part in making GlosSI work it's magic is End of Life.
You can read the announcement [here](https://docs.nefarius.at/projects/ViGEm/End-of-Life/)

As I don't think holding on to deprecated dependencies is a good way of moving forward, this effectively kills GlosSI as well since without ViGEm and HidHide GlosSI cannot function.

There will be a last deprecated version of ViGEm circumventing an issue stated in their announcement.
GlosSI will be updated a last time as well, providing this version bundled with it.
The GlosSI website will be taken down, however, you can then fin the last release, here on GitHub

### GlosSI won't be taken down or magically stop working, nor will it be unsafe to use. Just a maintenance stop.

---

I also want to take the opportunity to give a MASSIVE shoutout to @Nefarius , the creator of ViGEm, HidHide and many other awesome tools
Back when good old GloSC was just a cobbled together PoC using parts of the very old ScpToolkit he has been massively helpful and even shared ViGEm with me way before it was ready to be run on any machine that doesn't belong to a wizard (or should I say sorcerer?) like him.

--- 

Will GlosSI continue?

Probably. But most likely not in its current form.
@Nefarius wizardry is continuing and a successor to ViGEm is being worked on, but nothing has been publicly released, yet.

As I severely lack the time to properly maintain a project like GlosSI (as you probably have already noticed, I'm sure), I'm quite fond of the idea of rebooting the project yet again, once ViGEms successor becomes available.
However, I'm not sure if I find the time and motivation again to continue with GlosSI
Until I can (and want to) get my hands on it, the future is unsure...

I'll be back to silence for now then.
Thanks for all the support, it was a blast!

```
```
---

## How does it work? / What does it do?

GlosSI provides a target application that can be added as a "Non-Steam Game" to Steam.  
When launched, it redirects all configured controller inputs to a virtual system-level XBox360 controller.

Additionally, it provides the Steam Overlay in an (always on top) transparent window.

As a result, this brings full Steam-Input functionality to the desktop and any other application Steam-Input might not have worked before.

Games do not need to be launched using GlosSI.  
However, to ease managing multiple GlosSI shortcuts, there is also a GlosSI-Config application included.  
It allows one to create individual GlosSITarget configurations which can launch games for you, and easily add or remove them from Steam.

## What GlosSI is not

<div style="background: #5f000090; padding: 1em 0 0.5em 1em; border-radius: 2em; box-shadow: 0 8px 1px -2px rgba(0,0,0,.2),0 2px 2px 0 rgba(0,0,0,.14),0 1px 5px 0 rgba(0,0,0,.12)!important;">

- a replacement for Steams controller configuration tool.
- a Steam remote play / steam game streaming solution. (That being said, it **can** work, but is not guaranteed to.)  
  The experience when doing this is most likeley miserable; Thus there is **no support for this use case**.
- Old versions (GloSC, Global [S]team[C]ontroller) were never designed to be used with anything other than said controller, GlosSI can be used with any controller.
</div>

## Help and Support

If you're looking for a tutorial on how to use GlosSI check out the [usage section](https://glossi.flatspot.pictures/#usage) on the [GlosSI website](https://glossi.flatspot.pictures/) or check the [usage.md document](./docs/Usage.md)

---

**Get in touch on Discord!**  
Lots of kind and helpful people can be found there, happy to have a quick chat or answer support-requests  
[![Discord](https://img.shields.io/discord/368823110817808384.svg)](https://discord.gg/T9b4D5y)

## Other

Like my stuff? Hit me up [on twitter](https://twitter.com/Flatspotpics) or consider donating to my [PayPal](https://www.paypal.me/Flatspotpics)

GloSC got mentioned from Valve in the [Steam client beta change log on the 9. of January 2017](https://twitter.com/flatspotpics/status/818697837055770624)

GloSC/GlosSI is not affiliated with Valve, Steam, or any of their partners.

---

GlosSI is built using [Qt 6.X](https://www.qt.io/) and a fork of [SFML](http://www.sfml-dev.org/) for drawing the overlay

The system wide Xbox-Controller works via [ViGEm](https://vigem.org/projects/ViGEm/)
Device Hiding via [HidHide](https://vigem.org/projects/HidHide/)

For Building instructions refer to [BUILDING.md](./docs/BUILDING.md)

## License

```license
Copyright 2017-2023 Peter Repukat - FlatspotSoftware

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
