#pragma once
#include <thread>
#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <ViGEm/Client.h>
#include <Xinput.h>
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
    int controller_count_ = 0;
#ifdef _WIN32
    PVIGEM_CLIENT driver_;
    PVIGEM_TARGET vt_x360_[XUSER_MAX_COUNT]{};
    bool vigem_connected_;
    static void CALLBACK controllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData);
#endif

    std::thread controller_thread_;
};
