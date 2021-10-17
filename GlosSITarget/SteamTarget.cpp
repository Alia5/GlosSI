/*
Copyright 2021 Peter Repukat - FlatspotSoftware

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "SteamTarget.h"

SteamTarget::SteamTarget(int argc, char *argv[]) : window_([this] { run_ = false; })
SteamTarget::SteamTarget(int argc, char *argv[])
    : window_([this] { run_ = false; }),
      detector_([this](bool overlay_open) { onOverlayChanged(overlay_open); }), target_window_handle_(window_.getSystemHandle())
{
}

int SteamTarget::run()
{
    run_ = true;
    window_.setFpsLimit(90);
    while (run_) {
        detector_.update();
        window_.update();
    }
    return 1;
}

void SteamTarget::onOverlayChanged(bool overlay_open)
{
    window_.setClickThrough(!overlay_open);
    focusWindow(target_window_handle_);
}

void SteamTarget::focusWindow(WindowHandle hndl)
{
#ifdef _WIN32

    //MH_DisableHook(&GetForegroundWindow); // TODO: when GetForegroundWindow hooked, unhook!
    // store last focused window for later restore
    last_foreground_window_ = GetForegroundWindow();
    const DWORD fg_thread = GetWindowThreadProcessId(last_foreground_window_, nullptr);
    //MH_EnableHook(&GetForegroundWindow); // TODO: when GetForegroundWindow hooked, re-hook!

    // lot's of ways actually bringing our window to foreground...
    const DWORD current_thread = GetCurrentThreadId();
    AttachThreadInput(current_thread, fg_thread, TRUE);

    SetForegroundWindow(hndl);
    SetCapture(hndl);
    SetFocus(hndl);
    SetActiveWindow(hndl);
    EnableWindow(hndl, TRUE);

    AttachThreadInput(current_thread, fg_thread, FALSE);

    //try to forcefully set foreground window at least a few times
    sf::Clock clock;
    while (!SetForegroundWindow(hndl) && clock.getElapsedTime().asMilliseconds() < 20)
    {
        SetActiveWindow(hndl);
        Sleep(1);
    }

#endif
}
