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
#pragma once
#include <functional>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

class OverlayDetector {
  public:
    explicit OverlayDetector(
        std::function<void(bool)> overlay_changed = [](bool) {});
    void update();

  private:
    std::function<void(bool)> overlay_changed_;
#ifdef _WIN32
    bool overlay_open_ = false;
    int msg_count_ = 0;
#endif
};
