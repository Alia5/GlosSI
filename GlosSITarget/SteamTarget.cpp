#include "SteamTarget.h"

#include <iostream>

SteamTarget::SteamTarget(int argc, char* argv[]) : window_([this] { run_ = false; }) {

}

int SteamTarget::run() {
    run_ = true;
    window_.setFpsLimit(90);
    while (run_) {
        window_.update();
    }
    return 1;
}
