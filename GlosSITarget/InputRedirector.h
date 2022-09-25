/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

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
#pragma once
#include <thread>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <Xinput.h>
#include <ViGEm/Client.h>
#include <ViGEm/Util.h>
#endif

class InputRedirector {
  public:
    InputRedirector();
    ~InputRedirector();

    void run();
    void stop();

  private:
    void runLoop();

    static constexpr int start_delay_ms_ = 2000;
    bool run_ = false;
    int overlay_elem_id_ = -1;
#ifdef _WIN32
    PVIGEM_CLIENT driver_;

    // variables for overlay element; run in different thread
    static inline std::atomic<bool> enable_rumble_ = true;
    static inline std::atomic<bool> controller_settings_changed_ = false;

    bool vigem_connected_;

    PVIGEM_TARGET vt_pad_[XUSER_MAX_COUNT]{};
    static void CALLBACK x360ControllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData);
    void unplugVigemPad(int idx);

    static void CALLBACK ds4ControllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, DS4_LIGHTBAR_COLOR LightbarColor, LPVOID UserData);

#endif

    std::thread controller_thread_;
};
