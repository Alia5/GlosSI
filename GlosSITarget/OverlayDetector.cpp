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
#include "OverlayDetector.h"

#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <Psapi.h>
#define NOMINMAX
#include <Windows.h>
#endif


OverlayDetector::OverlayDetector(std::function<void(bool)> overlay_changed)
    : overlay_changed_(std::move(overlay_changed))
{
    auto addr_open = findFunctionByPattern(overlay_module_name_, open_func_sig_, open_func_mask_);
    auto addr_close = findFunctionByPattern(overlay_module_name_, close_func_sig_, close_func_mask_);

    spdlog::info("Overlay opened function: {0:x}", addr_open);
    spdlog::info("Overlay closed function: {0:x}", addr_close);


}

void OverlayDetector::update()
{
}

uint64_t OverlayDetector::findFunctionByPattern(
    const std::string_view &mod_name,
    const char pattern[],
    const std::string_view &mask) const
{
#ifdef _WIN32

    MODULEINFO mod_info = {NULL};
    const HMODULE mod = GetModuleHandleA(mod_name.data());

    if (mod == nullptr) {
        spdlog::error("{} not found!", overlay_module_name_);
        return 0;
    }
    GetModuleInformation(GetCurrentProcess(), mod, &mod_info, sizeof(MODULEINFO));

    auto base_addr = reinterpret_cast<uint64_t>(mod_info.lpBaseOfDll);
    if (base_addr == 0)
        return NULL;

    spdlog::debug("overlay module found at: {:x}", base_addr);

    const uint64_t mod_size = mod_info.SizeOfImage;
    const auto pat_length = mask.size();
    uint64_t pattern_addr = 0;

    for (uint64_t i = 0; i < mod_size - pat_length; i++) {
        bool found = true;
        for (uint64_t j = 0; j < pat_length; j++)
            found &= mask[j] == '?' || pattern[j] == *reinterpret_cast<char *>(base_addr + j + i);

        if (found)
            pattern_addr = base_addr + i;
    }
    if (pattern_addr == 0)
        return 0;

    spdlog::debug("signature found at: {:x}", pattern_addr);

    constexpr char start_fn_bytes[] = "\x40\x53";

    for (auto i = pattern_addr; i > base_addr; i--) {
        bool found = true;
        for (uint64_t j = 0; j < 2; j++)
            found &= start_fn_bytes[j] == *reinterpret_cast<char *>(i + j);

        if (found)
            return i;
    }
    return 0;

#else

#endif
}
