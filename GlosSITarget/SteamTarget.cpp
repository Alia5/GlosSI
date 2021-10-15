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

#include <iostream>

SteamTarget::SteamTarget(int argc, char *argv[]) : window_([this] { run_ = false; })
{
}

int SteamTarget::run()
{
    run_ = true;
    window_.setFpsLimit(90);
    while (run_) {
        window_.update();
    }
    return 1;
}
