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
#include "UnhookUtil.h"

#include <spdlog/spdlog.h>

#include "Settings.h"

void UnhookUtil::UnPatchHook(const std::string& name, HMODULE module)
{
    spdlog::trace("Patching \"{}\"...", name);

    BYTE* address = reinterpret_cast<BYTE*>(GetProcAddress(module, name.c_str()));
    if (!address) {
        spdlog::error("failed to unpatch \"{}\"", name);
    }
    std::string bytes;
    if (Settings::isWin10 && UNHOOK_BYTES_ORIGINAL_WIN10.contains(name)) {
        bytes = UNHOOK_BYTES_ORIGINAL_WIN10.at(name);
    }
    else {
        bytes = UNHOOK_BYTES_ORIGINAL_22000.at(name);
    }
    DWORD dw_old_protect, dw_bkup;
    const auto len = bytes.size();
    if (!VirtualProtect(address, len, PAGE_EXECUTE_READWRITE, &dw_old_protect)) { // Change permissions of memory..
        spdlog::error("Couldn't change permissions of memory for \"{}\"", name);
        return;
    }
    const auto opcode = *(address);
    if (!std::ranges::any_of(JUMP_INSTR_OPCODES, [&opcode](const auto& op) { return op == opcode; })) {
        spdlog::debug("\"{}\" Doesn't appear to be hooked, skipping!", name);
    }
    else {
        for (DWORD i = 0; i < len; i++) // unpatch Valve's hook
        {
            *(address + i) = bytes[i];
        }
        spdlog::trace("Unpatched \"{}\"", name);
    }
    VirtualProtect(address, len, dw_old_protect, &dw_bkup); // Revert permission change...
}
