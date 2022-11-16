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

#ifndef CONFIGAPP
#include <spdlog/spdlog.h>

#include "Settings.h"
#endif

void UnhookUtil::UnPatchHook(const std::string& name, HMODULE module)
{
#ifndef CONFIGAPP


    std::map<std::string, std::string> original_bytes_from_file;

    wchar_t* localAppDataFolder;
    std::filesystem::path configDirPath;
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &localAppDataFolder) != S_OK) {
        configDirPath = std::filesystem::temp_directory_path().parent_path().parent_path().parent_path();
    }
    else {
        configDirPath = std::filesystem::path(localAppDataFolder).parent_path();
    }

    configDirPath /= "Roaming";
    configDirPath /= "GlosSI";
    if (std::filesystem::exists(configDirPath)) {
        auto unhook_file_path = configDirPath / "unhook_bytes";
        if (std::filesystem::exists(unhook_file_path)) {

            std::ifstream ifile;
            ifile.open(unhook_file_path, std::ios::binary | std::ios::in);
            if (ifile.is_open()) {

                std::string funcName;
                char buff;
                do {
                    if (ifile.eof()) {
                        break;
                    }
                    ifile.read(&buff, sizeof(char));
                    if (buff != ':') {
                        funcName.push_back(buff);
                    } else {
                        char bytes[8];
                        ifile.read(bytes, sizeof(char) * 8);
                        ifile.read(&buff, sizeof(char)); // newline
                        original_bytes_from_file[funcName] = std::string(bytes, 8);
                        funcName = "";
                    }
                } while (!ifile.eof());

                ifile.close();
            }
        }
    }



    spdlog::trace("Patching \"{}\"...", name);

    BYTE* address = reinterpret_cast<BYTE*>(GetProcAddress(module, name.c_str()));
    if (!address) {
        spdlog::error("failed to unpatch \"{}\"", name);
    }
    std::string bytes;

    if (original_bytes_from_file.contains(name)) {
        bytes = original_bytes_from_file.at(name);
        spdlog::trace("Using originalBytes from file for {}", name);
    }
    else {
        if (Settings::isWin10 && UNHOOK_BYTES_ORIGINAL_WIN10.contains(name)) {
            bytes = UNHOOK_BYTES_ORIGINAL_WIN10.at(name);
        }
        else {
            bytes = UNHOOK_BYTES_ORIGINAL_22000.at(name);
        }
        spdlog::trace("Using fallback originalBytes for {}", name);
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
#endif
}

std::string UnhookUtil::ReadOriginalBytes(const std::string& name, const std::wstring& moduleName)
{
    auto module = LoadLibraryW(moduleName.c_str());
    auto address = reinterpret_cast<BYTE*>(GetProcAddress(module, name.c_str()));
    std::string res;
    res.resize(8);

    for (int i = 0; i < 8; i++) {
        res[i] = static_cast<char>(*(address + i));
    }
    return res;
}
