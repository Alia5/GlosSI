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
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QWindow>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")
#endif

#include "UIModel.h"
#include "qml/WinEventFilter.h"

#ifdef _WIN32
// Some undocument stuff to enable aero on win10 or higher...
enum AccentState
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_INVALID_STATE = 4
};
struct AccentPolicy
{

    AccentState AccentState;
    int AccentFlags;
    int GradientColor;
    int AnimationId;
};

enum WindowCompositionAttribute
{
    // ...
    WCA_ACCENT_POLICY = 19
    // ...
};
struct WindowCompositionAttributeData
{
    WindowCompositionAttribute Attribute;
    void* Data;
    int SizeOfData;
};

typedef HRESULT(__stdcall* PSetWindowCompositionAttribute)(HWND hwnd, WindowCompositionAttributeData* pattrs);

#endif


int main(int argc, char* argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    UIModel uimodel;
    engine.rootContext()->setContextProperty("uiModel", QVariant::fromValue(&uimodel));
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
#ifdef _WIN32

    // As a Qt.Frameless window sucks ass, and doesn't support Aerosnap and resizing and all that stuff is cumbersome on windows...
    // Use good old dwm...

    // First get window from QML.
    auto window = qobject_cast<QWindow*>(engine.rootObjects()[0]);
    const HWND hwnd = reinterpret_cast<HWND>(window->winId());
    // Clear it's title bar
    // NO!(!!) We wan't to keep WS_THICKFRAME, as that's what gives us a shadow, aerosnap, proper resizing, win11 round corners
    // ...and all that good stuff!
    auto style = GetWindowLong(hwnd, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(hwnd, GWL_STYLE, style);

    // Enable blurbehind (not needed?) anyway gives nice background on win7 and 8
    DWM_BLURBEHIND bb{ .dwFlags = DWM_BB_ENABLE, .fEnable = true, .hRgnBlur = nullptr };
    DwmEnableBlurBehindWindow(hwnd, &bb);

    //// undoc stuff for aero >= Win10
    //// leave for now... performance is crap!
    //// for now we have to live with unblurred transparency. lol
    //AccentPolicy accPol = { .AccentState = ACCENT_ENABLE_BLURBEHIND };
    //WindowCompositionAttributeData data = {
    //    .Attribute = WindowCompositionAttribute::WCA_ACCENT_POLICY,
    //    .Data = &accPol,
    //    .SizeOfData = sizeof(accPol)
    //};
    //auto user32dll = GetModuleHandle(L"user32.dll");
    //if (user32dll) {
    //    PSetWindowCompositionAttribute SetWindowCompositionAttribute = (
    //        reinterpret_cast<PSetWindowCompositionAttribute>(GetProcAddress(user32dll, "SetWindowCompositionAttribute"))
    //        );
    //    if (SetWindowCompositionAttribute)
    //    {
    //        SetWindowCompositionAttribute(hwnd, &data);
    //    }
    //}

    // extend the frame fully into the client area => draw all outside the window frame.
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // To Fix top window frame, install native event filter
    // basically the Qt. equivalent of having a own WndProc
    // for more info see WinEventFilter.h
    auto filter = std::make_shared<WinEventFilter>();
    app.installNativeEventFilter(filter.get());

    RECT rcClient;
    GetWindowRect(hwnd, &rcClient);

    // Inform the application of the frame change.
    SetWindowPos(hwnd,
        NULL,
        rcClient.left, rcClient.top,
        rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        SWP_FRAMECHANGED);

#endif

    return app.exec();
}
