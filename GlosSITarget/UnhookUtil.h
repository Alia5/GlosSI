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
#pragma once

#define NOMINMAX
#include <map>
#include <Windows.h>

#include <string>
#include <vector>

namespace UnhookUtil {
void UnPatchHook(const std::string& name, HMODULE module);

std::string ReadOriginalBytes(const std::string& name, const std::wstring& moduleName);

static inline const std::vector<uint8_t> JUMP_INSTR_OPCODES = {
    0xE9,
    0xE8,
    0xEB,
    0xEA,
    0xFF};

// Valve Hooks various functions and hides Gaming devices like this.
// To be able to query them, unpatch the hook with the original bytes...

// Bytes here are just fallbacks; originalbytes will get read from GlosSIConfig and stored in %APPDATA%\GlosSI\unhook_bytes

// 22000 ^= Windows build number
static inline const std::map<std::string, std::string> UNHOOK_BYTES_ORIGINAL_22000 = {
    {"SetupDiEnumDeviceInfo", "\x48\x89\x5C\x24\x08"},
    {"SetupDiGetClassDevsW", "\x48\x89\x5C\x24\x08"},
    {"HidD_GetPreparsedData", "\x48\x89\x5C\x24\x18"},
    {"HidP_GetCaps", "\x4C\x8B\xD1\x48\x85\xC9"},
    {"HidD_GetAttributes", "\x40\x53\x48\x83\xEC"},
    {"HidD_GetProductString", "\x48\x83\xEC\x48\x48"},
    {"HidP_GetUsages", "\x4C\x89\x4C\x24\x20"},
    {"HidP_GetData", "\x4C\x89\x44\x24\x18"},
    {"HidP_GetValueCaps", "\x48\x83\xEC\x48\x49"},
    {"HidP_GetUsageValue", "\x40\x53\x55\x56\x48"},
    {"HidP_GetButtonCaps", "\x48\x83\xEC\x48\x49"},
    // Valve hooks "CreateProcess" to detect child-processes
    {"CreateProcessW", "\x4C\x8B\xDC\x48\x83"},
};

// SetupApi.dll is different on Win10 than on Win11
static inline const std::map<std::string, std::string> UNHOOK_BYTES_ORIGINAL_WIN10 = {
    {"SetupDiEnumDeviceInfo", "\x40\x53\x56\x57\x41\x54\x41\x55"},
    {"SetupDiGetClassDevsW", "\x48\x8B\xC4\x48\x89\x58\x08"},
};


} // namespace UnhookUtil
