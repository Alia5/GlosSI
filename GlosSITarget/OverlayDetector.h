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


class OverlayDetector {
  public:
    explicit OverlayDetector(std::function<void(bool)> overlay_changed = [](bool) {});
    void update();

  private:
    std::function<void(bool)> overlay_changed_;


    uint64_t findFunctionByPattern(
        const std::string_view &mod_name,
        const char pattern[],
        const std::string_view &mask) const;

#ifdef _WIN32
    static constexpr std::string_view overlay_module_name_ = "GameOverlayRenderer64.dll";
    static constexpr char open_func_sig_[] = "\xC6\x41\x5C\x01\x48\x8D\x4C\x24\x30";
    static constexpr std::string_view open_func_mask_ = "xxxxxxxxx";
    static constexpr char close_func_sig_[] = "\xC6\x41\x5C\x00\x48\x8D\x4C\x24\x40";
    static constexpr std::string_view close_func_mask_ = "xxxxxxxxx";

#else

#endif
};
