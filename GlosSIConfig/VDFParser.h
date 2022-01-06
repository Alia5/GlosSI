/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

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

// "Shitty shortcuts.vdf Parser"�

#pragma once
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace VDFParser {
namespace crc {
template <typename CONT>
uint32_t calculate_crc(CONT container)
{
    uint32_t crc32_table[256];
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t ch = i;
        uint32_t crc = 0;
        for (size_t j = 0; j < 8; j++) {
            uint32_t b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b)
                crc = crc ^ 0xEDB88320;
            ch >>= 1;
        }
        crc32_table[i] = crc;
    }
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < container.size(); i++) {
        char ch = container.data()[i];
        uint32_t t = (ch ^ crc) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[t];
    }
    return ~crc;
}
} // namespace crc

static constexpr const char k_appid[] = {"appid"};
static constexpr const char k_appname[] = {"appname"};
static constexpr const char k_exe[] = {"exe"};
static constexpr const char k_StartDir[] = {"StartDir"};
static constexpr const char k_icon[] = {"icon"};
static constexpr const char k_ShortcutPath[] = {"ShortcutPath"};
static constexpr const char k_LaunchOptions[] = {"LaunchOptions"};
static constexpr const char k_IsHidden[] = {"IsHidden"};
static constexpr const char k_AllowDesktopConfig[] = {"AllowDesktopConfig"};
static constexpr const char k_AllowOverlay[] = {"AllowOverlay"};
static constexpr const char k_openvr[] = {"openvr"};
static constexpr const char k_Devkit[] = {"Devkit"};
static constexpr const char k_DevkitGameID[] = {"DevkitGameID"};
static constexpr const char k_DevkitOverrideAppID[] = {"DevkitOverrideAppID"};
static constexpr const char k_LastPlayTime[] = {"LastPlayTime"};
static constexpr const char k_tags[] = {"tags"};

enum VDFTypeId {
    StringList = 0,
    String,
    Number,
};

template <const char* const keyname, typename type, const uint8_t const _type_id>
struct VDFKeyPair {
    VDFKeyPair() {}
    explicit VDFKeyPair(type _value) : value(_value) {}
    static constexpr uint8_t _TID = _type_id;
    static constexpr const char* const _KEY = keyname;
    const uint8_t type_id = _TID;
    const char* const key = _KEY;
    type value;
    VDFKeyPair(const VDFKeyPair& other)
    {
        value = other.value;
    };
    VDFKeyPair(VDFKeyPair&& other)
    {
        value = std::move(other.value);
    };
    VDFKeyPair& operator=(const VDFKeyPair& other)
    {
        value = other.value;
        return *this;
    }
    VDFKeyPair& operator=(VDFKeyPair&& other)
    {
        value = std::move(other.value);
        return *this;
    }
};

template <uint8_t initialByte>
struct VDFIdx {
    VDFIdx(){};
    VDFIdx(const VDFIdx& other)
    {
        data[0] = other.data[0];
        data[1] = other.data[1];
    };
    VDFIdx(VDFIdx&& other)
    {
        data[0] = std::move(other.data[0]);
        data[1] = std::move(other.data[1]);
    };
    VDFIdx(int idx)
    {
        if (idx > 99) {
            data[0] = initialByte;
            data[1] = 0;
        }
        else if (idx < 10) {
            data[1] = std::to_string(idx).c_str()[0];
        }
        else {
            auto meh = std::to_string(idx).c_str();
            data[0] = meh[0] + initialByte;
            data[1] = meh[1];
        }
    }
    char data[2] = {initialByte, 0x0};
    operator int() const
    {
        if (data[0] == initialByte) {
            int res = 0;
            std::from_chars(&data[1], &data[1], res);
            return res;
        }
        int res = 0;
        char copy[2] = {data[0] - initialByte, data[1]};
        std::from_chars(&copy[0], &copy[1], res);
        return res;
    }

    VDFIdx& operator=(const VDFIdx& other)
    {
        data[0] = other.data[0];
        data[1] = other.data[1];
        return *this;
    }
    VDFIdx& operator=(VDFIdx&& other)
    {
        data[0] = std::move(other.data[0]);
        data[1] = std::move(other.data[1]);
        return *this;
    }
};

struct ShortcutTag {
    ShortcutTag(){};
    ShortcutTag(const ShortcutTag& other)
    {
        value = other.value;
    };
    ShortcutTag(ShortcutTag&& other)
    {
        value = std::move(other.value);
    };
    VDFIdx<0x01> idx; // I Hope this is how it works... See VDFIdx
    std::string value;
    const uint16_t end_marker = 0x0808;

    ShortcutTag& operator=(const ShortcutTag& other)
    {
        value = other.value;
        return *this;
    }
    ShortcutTag& operator=(ShortcutTag&& other)
    {
        value = std::move(other.value);
        return *this;
    }
};

struct Shortcut {
    VDFIdx<0x00> idx;
    VDFKeyPair<k_appid, uint32_t, VDFTypeId::Number> appId{0x000000};
    VDFKeyPair<k_appname, std::string, VDFTypeId::String> appName{""};
    VDFKeyPair<k_exe, std::string, VDFTypeId::String> exe{"\"\""};                 // Qouted
    VDFKeyPair<k_StartDir, std::string, VDFTypeId::String> StartDir{"\"\""};       // Qouted
    VDFKeyPair<k_icon, std::string, VDFTypeId::String> icon{""};                   // Qouted or empty
    VDFKeyPair<k_ShortcutPath, std::string, VDFTypeId::String> ShortcutPath{""};   // Qouted or empty?
    VDFKeyPair<k_LaunchOptions, std::string, VDFTypeId::String> LaunchOptions{""}; // UNQOUTED or empty
    VDFKeyPair<k_IsHidden, uint32_t, VDFTypeId::Number> IsHidden{0};
    VDFKeyPair<k_AllowDesktopConfig, uint32_t, VDFTypeId::Number> AllowDesktopConfig{1};
    VDFKeyPair<k_AllowOverlay, uint32_t, VDFTypeId::Number> AllowOverlay{1};
    VDFKeyPair<k_openvr, uint32_t, VDFTypeId::Number> openvr{0};
    VDFKeyPair<k_Devkit, uint32_t, VDFTypeId::Number> Devkit{0};
    VDFKeyPair<k_DevkitGameID, std::string, VDFTypeId::String> DevkitGameID{""};
    VDFKeyPair<k_DevkitOverrideAppID, uint32_t, VDFTypeId::Number> DevkitOverrideAppID{0}; //
    VDFKeyPair<k_LastPlayTime, uint32_t, VDFTypeId::Number> LastPlayTime{0};               //
    VDFKeyPair<k_tags, std::vector<ShortcutTag>, VDFTypeId::StringList> tags{};
    Shortcut& operator=(const Shortcut& other)
    {
        idx = other.idx;
        appId = other.appId;
        appName = other.appName;
        exe = other.exe;
        StartDir = other.StartDir;
        icon = other.icon;
        ShortcutPath = other.ShortcutPath;
        LaunchOptions = other.LaunchOptions;
        LaunchOptions = other.LaunchOptions;
        IsHidden = other.IsHidden;
        AllowDesktopConfig = other.AllowDesktopConfig;
        AllowOverlay = other.AllowOverlay;
        openvr = other.openvr;
        Devkit = other.Devkit;
        DevkitGameID = other.DevkitGameID;
        DevkitOverrideAppID = other.DevkitOverrideAppID;
        LastPlayTime = other.LastPlayTime;
        tags = other.tags;
        return *this;
    }
    //std::wstring to_json()
    //{
    //    std::wstring res = L"{";
    //    res += L"idx: " + std::to_wstring(idx.operator int()) + L",\n";
    //    res += L"appId: " + std::to_wstring(appId.value) + L",\n";
    //    res += L"appName: " + std::filesystem::path(appName.value).wstring() + L",\n";
    //    res += L"StartDir: " + std::filesystem::path(StartDir.value).wstring() + L",\n";
    //    res += L"ShortcutPath: " + std::filesystem::path(ShortcutPath.value).wstring() + L",\n";
    //    res += L"LaunchOptions: " + std::filesystem::path(LaunchOptions.value).wstring() + L",\n";
    //    res += L"IsHidden: " + (IsHidden.value ? L"true" : L"false") + L",\n";
    //    res += L"AllowDesktopConfig: " + (AllowDesktopConfig.value ? L"true" : L"false") + L",\n";
    //    res += L"idx: " + std::to_wstring(appId.value) + L",\n";
    //    res += L"}";
    //    return res;
    //}
};

struct VDFFile {
    VDFFile(){};
    VDFFile(const VDFFile& other)
    {
        shortcuts = other.shortcuts;
    };
    VDFFile(VDFFile&& other)
    {
        shortcuts = std::move(other.shortcuts);
    };
    const uint8_t first_byte = 0x00;
    const std::string identifier = "shortcuts";
    std::vector<Shortcut> shortcuts;
    const uint16_t end_marker = 0x0808;
    VDFFile& operator=(const VDFFile& other)
    {
        shortcuts = other.shortcuts;
        return *this;
    }
    VDFFile& operator=(VDFFile&& other)
    {
        shortcuts = std::move(other.shortcuts);
        return *this;
    }
    //std::wstring to_json()
    //{
    //    std::wstring res = L"[";



    //    res += L"]";
    //    return res;
    //}
};

class Parser {
  private:
    static inline std::ifstream ifile;
    static inline std::ofstream ofile;

    template <typename typ, typename size>
    static inline auto readVDFBuffer(typ& buff, size sz)
    {
        if (ifile.eof()) {

            return;
        }
        ifile.read((char*)buff, sz);
    }

    template <typename typ>
    static inline auto readVDFValue()
    {
        uint8_t buff[sizeof(typ)];
        ifile.read((char*)buff, sizeof(typ));
        return *reinterpret_cast<typ*>(buff);
    }

    static inline std::string readVDFString()
    {
        std::string str;
        char ch = '\x0';
        do {
            if (ifile.eof()) {
                return str;
            }
            ifile.read(&ch, sizeof(char));
            if (ch != '\x0')
                str.push_back(ch);
        } while (ch != '\x0');
        return str;
    }

  public:
    static inline uint32_t calculateAppId(const Shortcut& shortcut)
    {
        std::string buff = shortcut.exe.value + shortcut.appName.value;
        auto checksum = crc::calculate_crc(buff);
        return checksum | 0x80000000;
    }

    static inline VDFFile parseShortcuts(std::filesystem::path path)
    {
        VDFFile vdffile;

        ifile.open(path.string(), std::ios::binary | std::ios::in);
        if (!ifile.is_open()) {
            return {};
        }

        auto firsty = readVDFValue<uint8_t>();
        if (vdffile.first_byte != firsty) {
            // TODO: invalid
            ifile.close();
            return vdffile;
        }

        auto headername = readVDFString();
        if (vdffile.identifier != headername) {
            // TODO: invalid
            ifile.close();
            return vdffile;
        }

        char buff[3];
        while (true) {
            if (ifile.eof()) {
                break;
            }
            readVDFBuffer(buff, 3); // 2 bytes idx, 0x00 delmiter
            if (buff[0] == 0x08 && buff[1] == 0x08) {
                break;
            }
            Shortcut shortcut;
            shortcut.idx.data[0] = buff[0];
            shortcut.idx.data[1] = buff[1];
            while (true) // TODO;
            {
                if (ifile.eof()) {
                    break;
                }
                VDFTypeId tid = static_cast<VDFTypeId>(readVDFValue<uint8_t>());
                if (tid == 0x08) {
                    auto nextbyte = readVDFValue<uint8_t>();
                    if (nextbyte == 0x08) {
                        break;
                    }
                    else {
                        // WTF?!
                        // TODO:
                        throw std::exception("WTF");
                    }
                }
                auto key = readVDFString();
                if ((tid == 0x08 && key[0] == 0x08) || key == "\x08\x08") {
                    break;
                }
                if (key == shortcut.appId.key) {
                    shortcut.appId.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.appName.key) {
                    shortcut.appName.value = readVDFString();
                    continue;
                }
                if (key == shortcut.exe.key) {
                    shortcut.exe.value = readVDFString();
                    continue;
                }
                if (key == shortcut.StartDir.key) {
                    shortcut.StartDir.value = readVDFString();
                    continue;
                }
                if (key == shortcut.icon.key) {
                    shortcut.icon.value = readVDFString();
                    continue;
                }
                if (key == shortcut.ShortcutPath.key) {
                    shortcut.ShortcutPath.value = readVDFString();
                    continue;
                }
                if (key == shortcut.LaunchOptions.key) {
                    shortcut.LaunchOptions.value = readVDFString();
                    continue;
                }
                if (key == shortcut.IsHidden.key) {
                    shortcut.IsHidden.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.AllowDesktopConfig.key) {
                    shortcut.AllowDesktopConfig.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.AllowOverlay.key) {
                    shortcut.AllowOverlay.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.openvr.key) {
                    shortcut.openvr.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.Devkit.key) {
                    shortcut.Devkit.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.DevkitGameID.key) {
                    shortcut.DevkitGameID.value = readVDFString();
                    continue;
                }
                if (key == shortcut.DevkitOverrideAppID.key) {
                    shortcut.DevkitOverrideAppID.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.LastPlayTime.key) {
                    shortcut.LastPlayTime.value = readVDFValue<uint32_t>();
                    continue;
                }
                if (key == shortcut.tags.key) {
                    // TODO: read tags
                    ShortcutTag tag;
                    while (true) {
                        if (ifile.eof()) {
                            break;
                        }
                        char tbuff[2];
                        readVDFBuffer(tbuff, 2); // 2 bytes idx
                        if (tbuff[0] == 0x08 && tbuff[1] == 0x08) {
                            ifile.seekg(-2, std::ios_base::cur);
                            break;
                        }
                        tag.idx.data[0] = tbuff[0];
                        tag.idx.data[1] = tbuff[1];
                        ifile.seekg(1, std::ios_base::cur);
                        tag.value = readVDFString();
                        shortcut.tags.value.push_back(tag);
                    }
                    continue;
                }
            }
            if (!(shortcut.idx.data[0] == 0x0 && shortcut.idx.data[1] == 0x0)) {
                vdffile.shortcuts.push_back(shortcut);
            }
        }

        ifile.close();

        return vdffile;
    }

    static inline bool writeShortcuts(std::filesystem::path path, const VDFFile& vdffile)
    {
        ofile.open(path.string(), std::ios::binary | std::ios::out);
        if (!ofile.is_open()) {
            return false;
        }
        ofile.write((char*)&vdffile.first_byte, 1);
        ofile.write(vdffile.identifier.data(), vdffile.identifier.length());
        ofile.write("\x00", 1);
        for (auto& shortcut : vdffile.shortcuts) {
            ofile.write(shortcut.idx.data, 2);
            ofile.write("\x00", 1);
            //
            ofile.write((char*)&shortcut.appId.type_id, 1);
            ofile.write(shortcut.appId.key, 6);
            ofile.write((char*)&shortcut.appId.value, 4);

            //
            ofile.write((char*)&shortcut.appName.type_id, 1);
            ofile.write(shortcut.appName.key, 8);
            ofile.write(shortcut.appName.value.data(), shortcut.appName.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.exe.type_id, 1);
            ofile.write(shortcut.exe.key, 4);
            ofile.write(shortcut.exe.value.data(), shortcut.exe.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.StartDir.type_id, 1);
            ofile.write(shortcut.StartDir.key, 9);
            ofile.write(shortcut.StartDir.value.data(), shortcut.StartDir.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.icon.type_id, 1);
            ofile.write(shortcut.icon.key, 5);
            ofile.write(shortcut.icon.value.data(), shortcut.icon.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.ShortcutPath.type_id, 1);
            ofile.write(shortcut.ShortcutPath.key, 13);
            ofile.write(shortcut.ShortcutPath.value.data(), shortcut.ShortcutPath.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.LaunchOptions.type_id, 1);
            ofile.write(shortcut.LaunchOptions.key, 14);
            ofile.write(shortcut.LaunchOptions.value.data(), shortcut.LaunchOptions.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.IsHidden.type_id, 1);
            ofile.write(shortcut.IsHidden.key, 9);
            ofile.write((char*)&shortcut.IsHidden.value, 4);

            //
            ofile.write((char*)&shortcut.AllowDesktopConfig.type_id, 1);
            ofile.write(shortcut.AllowDesktopConfig.key, 19);
            ofile.write((char*)&shortcut.AllowDesktopConfig.value, 4);

            //
            ofile.write((char*)&shortcut.AllowOverlay.type_id, 1);
            ofile.write(shortcut.AllowOverlay.key, 13);
            ofile.write((char*)&shortcut.AllowOverlay.value, 4);

            //
            ofile.write((char*)&shortcut.openvr.type_id, 1);
            ofile.write(shortcut.openvr.key, 7);
            ofile.write((char*)&shortcut.openvr.value, 4);

            //
            ofile.write((char*)&shortcut.Devkit.type_id, 1);
            ofile.write(shortcut.Devkit.key, 7);
            ofile.write((char*)&shortcut.Devkit.value, 4);

            //
            ofile.write((char*)&shortcut.DevkitGameID.type_id, 1);
            ofile.write(shortcut.DevkitGameID.key, 13);
            ofile.write(shortcut.DevkitGameID.value.data(), shortcut.DevkitGameID.value.length());
            ofile.write("\x00", 1);

            //
            ofile.write((char*)&shortcut.DevkitOverrideAppID.type_id, 1);
            ofile.write(shortcut.DevkitOverrideAppID.key, 20);
            ofile.write((char*)&shortcut.DevkitOverrideAppID.value, 4);

            //
            ofile.write((char*)&shortcut.LastPlayTime.type_id, 1);
            ofile.write(shortcut.LastPlayTime.key, 13);
            ofile.write((char*)&shortcut.LastPlayTime.value, 4);

            //
            ofile.write((char*)&shortcut.tags.type_id, 1);
            ofile.write(shortcut.tags.key, 5);
            for (auto& tag : shortcut.tags.value) {
                ofile.write(tag.idx.data, 2);
                ofile.write("\x00", 1);
                ofile.write(tag.value.data(), tag.value.length());
                ofile.write("\x00", 1);
            }
            ofile.write("\x08\x08", 2);
        }
        ofile.write("\x08\x08", 2);
        ofile.close();
        return true;
    }
};
} // namespace VDFParser
