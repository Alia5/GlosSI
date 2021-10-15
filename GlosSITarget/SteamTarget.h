#pragma once

#include "TargetWindow.h"

class SteamTarget
{
public:
    explicit SteamTarget(int argc, char* argv[]);
    int run();

private:
    bool run_ = false;
    TargetWindow window_;
};

