# GloSC

GlosSC or Global SteamController is a little set of tools that allows you to use your SteamController as a systemwide XInput-controller alongside a systemwide (borderless window) Steam overlay with **per application bindings and working rumble emulation.**

The systemwide Xbox-Controller works via [ViGEm](https://github.com/nefarius/ViGEm)


GloSC consists of:
 
 * A main application ("GloSC") allowing you to create and configure new shortcuts or "SteamTargets" and (at a later date) automatically add them to Steam
 * The "GloSC-GameLauncher" which can launch Win32 and UWP games and mainly exists to work around dual overlays
 * The "SteamTarget" which does most of the magic - Showing the overlay to the user as well as talking to the ViGEm-driver for systemwide Controller emulation
 * A little config window for the SteamTarget, "SteamTargetUserWindow" which allows the user to close the SteamTarget as well as to configure a few basic things while a SteamTarget is running


Like my stuff? Consider donating to my [PayPal](https://www.paypal.me/Flatspotpics)

_Work in progress_


## License

```
Copyright 2016 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
