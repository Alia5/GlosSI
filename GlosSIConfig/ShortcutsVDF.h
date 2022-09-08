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
#pragma once
#include <any>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <regex>
#include <ranges>

#include <fifo_map.hpp>

namespace VDFParser {

namespace internal {
    constexpr unsigned int str2int(const char* str, int h = 0)
    {
        return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
    }
}


namespace crc {
template <typename CONT>
uint32_t calculate_crc(CONT container)
{
    uint32_t crc32_table[256];
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t ch = i;
        uint32_t crc = 0;
        for (size_t j = 0; j < 8; j++) {
            const uint32_t b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b)
                crc = crc ^ 0xEDB88320;
            ch >>= 1;
        }
        crc32_table[i] = crc;
    }
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < container.size(); i++) {
        const char ch = container.data()[i];
        const uint32_t t = (ch ^ crc) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[t];
    }
    return ~crc;
}
} // namespace crc

enum VDFTypeId {
    Map = 0,
    String,
    Number,
    EndMarker = 0x08
};

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
static constexpr const char k_FlatpakAppID[] = {"FlatpakAppID"};
static constexpr const char k_tags[] = {"tags"};

static const std::string SHORTCUTS_IDENTIFIER = "shortcuts";

static inline const std::vector<std::string> VDF_KEYS = {
    k_appid, 
    k_appname, 
    k_exe, 
    k_StartDir,
    k_icon,
    k_ShortcutPath,
    k_LaunchOptions, 
    k_IsHidden,
    k_AllowDesktopConfig,
    k_AllowOverlay,
    k_openvr, 
    k_Devkit, 
    k_DevkitGameID, 
    k_DevkitOverrideAppID, 
    k_LastPlayTime,
    k_FlatpakAppID,
    k_tags
};
struct VDFValue {
    VDFTypeId type = EndMarker;
    std::any value{};

    [[nodiscard]] std::string to_json() const
    {
        if (!value.has_value()) {
            return "null";
        }
        switch (type) {
        case Number:
            return std::to_string(std::any_cast<uint32_t>(value));
        case String:
            return "\"" + std::regex_replace(std::regex_replace(std::any_cast<std::string>(value), std::regex(R"(\\)"), R"(\\)"), std::regex(R"(")"), R"(\")") + "\"";
        case Map: {
            const auto& map = std::any_cast<nlohmann::fifo_map<std::string, VDFValue>>(value);
            std::string res = "{\n";
            for (const auto& [key, v] : map) {
                res += "\"" + key + "\": " + v.to_json() + ",\n";
            }
            if (res.ends_with(",\n")) {
                res.pop_back();
                res.pop_back();
            }
            return res + "\n}";
        }
        case EndMarker:
            // WTF?!
            return "\"EndMarker\"";
        default:
            return "null";
        }
    }

    operator std::string() const
    {
        return to_json();
    }
};

static inline const nlohmann::fifo_map<std::string, VDFValue> DEFAULT_SHORTCUT_MAP = {
    { k_appid,                  {VDFTypeId::Number, 0}},
    { k_appname,                {VDFTypeId::String, ""}},
    { k_exe,                    {VDFTypeId::String, "\"\""}},
    { k_StartDir,               {VDFTypeId::String, "\"\""}},
    { k_icon,                   {VDFTypeId::String, "\"\""}},
    { k_ShortcutPath,           {VDFTypeId::String, "\"\""}},
    { k_LaunchOptions,          {VDFTypeId::String, ""}},
    { k_IsHidden,               {VDFTypeId::Number, ""}},
    { k_AllowDesktopConfig,     {VDFTypeId::Number, 0}},
    { k_AllowOverlay,           {VDFTypeId::Number, 1}},
    { k_openvr,                 {VDFTypeId::Number, 0}},
    { k_Devkit,                 {VDFTypeId::Number, 0}},
    { k_DevkitGameID,           {VDFTypeId::String, ""}},
    { k_DevkitOverrideAppID,    {VDFTypeId::Number, 0}},
    { k_LastPlayTime,           {VDFTypeId::Number, ""}},
    { k_FlatpakAppID,           {VDFTypeId::String, ""}},
    { k_tags,                   {VDFTypeId::Map, nlohmann::fifo_map < std::string, VDFValue >()}},
};

struct Shortcut {
    uint32_t appid{};
    std::string appname;
    std::string exe;
    std::string StartDir;
    std::string icon;
    std::string ShortcutPath;
    std::string LaunchOptions;
    uint32_t IsHidden{};
    uint32_t AllowDesktopConfig{};
    uint32_t AllowOverlay{};
    uint32_t openvr{};
    uint32_t Devkit{};
    std::string DevkitGameID;
    uint32_t DevkitOverrideAppID{};
    uint32_t LastPlayTime{};
    std::string FlatpakAppID;
    std::vector<std::string> tags;

    nlohmann::fifo_map<std::string, VDFValue> unsupported_keys;

    Shortcut() = default;
    explicit Shortcut(const nlohmann::fifo_map<std::string, VDFValue>& vdf_map) : Shortcut()
    {
        for (const auto& [key, value] : (vdf_map.empty() ? DEFAULT_SHORTCUT_MAP : vdf_map)) {
            switch (value.type) {
            case Number: {
                switch (internal::str2int(key.c_str())) {
                case internal::str2int(k_appid):
                    appid = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_IsHidden):
                    IsHidden = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_AllowDesktopConfig):
                    AllowDesktopConfig = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_AllowOverlay):
                    AllowOverlay = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_openvr):
                    openvr = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_Devkit):
                    Devkit = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_DevkitOverrideAppID):
                    DevkitOverrideAppID = std::any_cast<uint32_t>(value.value);
                    break;
                case internal::str2int(k_LastPlayTime):
                    LastPlayTime = std::any_cast<uint32_t>(value.value);
                    break;
                default:
                    unsupported_keys[key] = value;
                    break;
                }
                break;
            case String: {
                switch (internal::str2int(key.c_str())) {
                case internal::str2int(k_appname):
                    appname = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_exe):
                    exe = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_StartDir):
                    StartDir = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_icon):
                    icon = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_ShortcutPath):
                    ShortcutPath = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_LaunchOptions):
                    LaunchOptions = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_DevkitGameID):
                    DevkitGameID = std::any_cast<std::string>(value.value);
                    break;
                case internal::str2int(k_FlatpakAppID):
                    FlatpakAppID = std::any_cast<std::string>(value.value);
                    break;
                default:
                    unsupported_keys[key] = value;
                    break;
                }
                break;
            }
            case Map: {
                switch (internal::str2int(key.c_str())) {
                case internal::str2int(k_tags): {
                    for (const auto& [type, tag] : std::any_cast<nlohmann::fifo_map<std::string, VDFValue>>(value.value) | std::views::values) {
                        if (type == String) {
                            tags.push_back(std::any_cast<std::string>(tag));
                        }
                    }
                    break;
                }
                default:
                    unsupported_keys[key] = value;
                    break;
                }
                break;
            }
            }
            }
        }
    }

    [[nodiscard]] uint32_t calculateAppId() const
    {
        const auto checksum = crc::calculate_crc(exe + appname);
        return checksum | 0x80000000;
    }

    operator VDFValue() const
    {
        nlohmann::fifo_map<std::string, VDFValue> value;
        value[k_appid] = {Number, appid ? appid : calculateAppId()};
        value[k_appname] = {String, appname};
        value[k_exe] = {String, exe};
        value[k_StartDir] = {String, StartDir};
        value[k_icon] = {String, icon};
        value[k_ShortcutPath] = {String, ShortcutPath};
        value[k_LaunchOptions] = {String, LaunchOptions};
        value[k_IsHidden] = {Number, IsHidden};
        value[k_AllowDesktopConfig] = {Number, AllowDesktopConfig};
        value[k_AllowOverlay] = {Number, AllowOverlay};
        value[k_openvr] = {Number, openvr};
        value[k_Devkit] = {Number, Devkit};
        value[k_DevkitGameID] = {String, DevkitGameID};
        value[k_DevkitOverrideAppID] = {Number, DevkitOverrideAppID};
        value[k_LastPlayTime] = {Number, LastPlayTime};
        value[k_FlatpakAppID] = {String, FlatpakAppID};

        nlohmann::fifo_map<std::string, VDFValue> tag_map;
        for (size_t i = 0; i < tags.size(); i++) {
            tag_map[std::to_string(i)] = {String,tags[i]};
        }
        value[k_tags] = {Map, tag_map};

        for (const auto& [key, v] : unsupported_keys) {
            value[key] = v;
        }

        return {Map, value};
    }
};

class Parser {
  private:
    static inline std::ifstream ifile;
    static inline std::ofstream ofile;

    template <typename typ>
    static inline auto readVDFValue()
    {
        uint8_t buff[sizeof(typ)];
        ifile.read((char*)buff, sizeof(typ));
        return *reinterpret_cast<typ*>(buff);
    }

    template <>
    static inline auto readVDFValue<std::string>()
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

    template <>
    static inline auto readVDFValue<nlohmann::fifo_map<std::string, VDFValue>>()
    {
        auto res = nlohmann::fifo_map<std::string, VDFValue>();
        while (true) {
            const auto& [key, value] = readVDFValue();
            if (value.type == EndMarker) {
                return res;
            }
            res[key] = value;
        }
    }

    static inline std::pair<std::string, VDFValue> readVDFValue()
    {
        auto res = std::pair<std::string, VDFValue>("", VDFValue(EndMarker, nullptr));

        if (ifile.eof()) {
            return res;
        }
        const auto tid = static_cast<VDFTypeId>(readVDFValue<uint8_t>());
        if (tid == EndMarker) {
            return res;
        }
        res.second.type = tid;
        res.first = readVDFValue<std::string>();
        switch (tid) {
        case VDFTypeId::Map:
            res.second.value = readVDFValue<nlohmann::fifo_map<std::string, VDFValue>>();
            break;
        case VDFTypeId::Number:
            res.second.value = readVDFValue<uint32_t>();
            break;
        case VDFTypeId::String:
            res.second.value = readVDFValue<std::string>();
            break;
        default:
            throw std::exception("VDF: Unknown TypeID");
            break;
        }

        return res;
    }

    template <typename typ>
    static inline auto writeVDFValue(typ v)
    {
        ofile.write((char*)&v, sizeof(typ));
    }

    template <>
    static inline auto writeVDFValue<std::string>(std::string v)
    {
        ofile.write(v.data(), v.length());
        ofile.write("\x00", 1);
    }

    template <>
    static inline auto writeVDFValue<nlohmann::fifo_map<std::string, VDFValue>>(nlohmann::fifo_map<std::string, VDFValue> v)
    {
        for (const auto& pair : v) {
            writeVDFValue(pair);
        }
        ofile.write("\x08", 1);
    }

    static inline void writeVDFValue(const std::pair<const std::string, VDFValue>& value)
    {
        ofile.write((char*)(&value.second.type), 1);
        writeVDFValue(value.first);
        switch (value.second.type) {
        case Map:
            writeVDFValue(std::any_cast<nlohmann::fifo_map<std::string, VDFValue>>(value.second.value));
            break;
        case Number:
            writeVDFValue(std::any_cast<uint32_t>(value.second.value));
            break;
        case String:
            writeVDFValue(std::any_cast<std::string>(value.second.value));
            break;
        default:
            throw std::exception("VDF: Unknown TypeID");
            break;
        }
    }

  public:
    template <typename LogStream = std::ostream>
    static inline std::vector<Shortcut> parseShortcuts(const std::filesystem::path& path, LogStream l = std::cout)
    {

        ifile.open(path, std::ios::binary | std::ios::in);
        if (!ifile.is_open()) {
            return {};
        }

        const auto& shortcutsVDF = readVDFValue();
        ifile.close();
        if (shortcutsVDF.second.type != Map || shortcutsVDF.first != SHORTCUTS_IDENTIFIER) {
            throw std::exception("invalid shortcuts file!");
        }

        const auto& v = std::any_cast<nlohmann::fifo_map<std::string, VDFValue>>(shortcutsVDF.second.value);

        std::vector<Shortcut> shortcuts;
        for (const auto& [type, sc] : v | std::views::values) {
            if (type != Map) {
                // throw std::exception("invalid shortcuts file!");
                // TODO: warn unsupported
                l << "unsupported or invalid shortcuts-file!";
                continue;
            }
            shortcuts.emplace_back(std::any_cast<nlohmann::fifo_map<std::string, VDFValue>>(sc));
        }

        return shortcuts;
    }

    template <typename LogStream = std::ostream>
    static inline bool writeShortcuts(const std::filesystem::path& path, const std::vector<Shortcut>& shortcuts, LogStream l = std::cout)
    {
        const auto backupFileName = path.wstring() + L".bak";
        if (std::filesystem::exists(path) && !std::filesystem::exists(backupFileName)) {
            l << "No shortcuts backup detected... Creating now...";
            const auto copied = std::filesystem::copy_file(path, backupFileName, std::filesystem::copy_options::update_existing);
            l << "failed to copy shortcuts.vdf to backup!";
        }

        ofile.open(path.wstring(), std::ios::binary | std::ios::out);
        if (!ofile.is_open()) {
            return false;
        }
        nlohmann::fifo_map<std::string, VDFValue> shortcuts_map;
        for (size_t i = 0; i < shortcuts.size(); i++) {
            shortcuts_map[std::to_string(i)] = shortcuts[i];
        }

        writeVDFValue({SHORTCUTS_IDENTIFIER, {Map, shortcuts_map}});
        ofile.write("\x08", 1);
        ofile.close();
        return true;
    }
};
} // namespace VDFParser
