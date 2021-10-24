#pragma once
#include <QAbstractNativeEventFilter>
#include <QString>
#define NOMINMAX
#include <Windows.h>

class WinEventFilter : public QAbstractNativeEventFilter
{
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
            if (msg == WM_NCCALCSIZE)
            {
                auto sz = reinterpret_cast<LPNCCALCSIZE_PARAMS>(lParam);
                sz->rgrc[0].top -= 6;
            }
        }
        return false;
    }
};
