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
#include "OverlayDetector.h"

#include <spdlog/spdlog.h>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

OverlayDetector::OverlayDetector(std::function<void(bool)> overlay_changed)
    : overlay_changed_(std::move(overlay_changed))
{
}

void OverlayDetector::update()
{
#ifdef _WIN32
    // Steam hooks into Windows messages
    // as long as the overlay is open, every msg (except for input messages?)
    // get's the message field set to `0`
    // to detect the overlay, just exploit this.
    // if the message is 0 3 frames consecutively -> overlay is open
    // not -> closed
    //
    // i'm guessing something very similar is done on linux
    // however
    // reversing on linux SUUUUUUUCKS!
    MSG msg;
    // okey to use nullptr as hwnd. get EVERY message
    if (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE)) {
        // filter out some messages as not all get altered by steam...
        if (msg.message < 1000 && msg.message > 0) {
            return;
        }
        if (msg.message == 0 && !overlay_open_) {
            msg_count_++;
            if (msg_count_ >= 3) {
                msg_count_ = 0;
                overlay_open_ = true;
                spdlog::info("Overlay opened");
                overlay_changed_(overlay_open_);
            }
        }
        if (msg.message != 0 && overlay_open_) {
            msg_count_++;
            if (msg_count_ >= 3) {
                msg_count_ = 0;
                overlay_open_ = false;
                spdlog::info("Overlay closed");
                overlay_changed_(overlay_open_);
            }
        }
    }
#endif
}