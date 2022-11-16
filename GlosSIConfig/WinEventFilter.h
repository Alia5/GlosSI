/*
Copyright 2021-2023 Peter Repukat - FlatspotSoftware

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
#include <QAbstractNativeEventFilter>
#include <QString>
#define NOMINMAX
#include <Windows.h>

class WinEventFilter : public QAbstractNativeEventFilter {
  public:
    WinEventFilter() = default;

    /*
     * When having the DWM frame fully extended into client area
     * ever since WIN10 a 6px border is displayed on top.
     * to remove that one has to catch the WM_NCCALCSIZE event and re-calculate the window-rect
     * https://stackoverflow.com/a/2135120/5106063
     * https://docs.microsoft.com/en-us/windows/win32/dwm/customframe
     */
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override
    {
        if (QString(eventType) == "windows_generic_MSG") {
            auto msg = static_cast<MSG*>(message)->message;
            auto lParam = static_cast<MSG*>(message)->lParam;
            auto hwnd = static_cast<MSG*>(message)->hwnd;
            if (msg == WM_NCCALCSIZE) {
                auto sz = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam);
                sz->rgrc[0].top -= 6;
            }
        }
        return false;
    }
};
