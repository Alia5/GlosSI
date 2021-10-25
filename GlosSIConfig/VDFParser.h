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
#include <filesystem>
#include <QString>
#include <QVariant>
#include <QList>

namespace VDFParser
{
    static constexpr const char k_appId[] = { "appId" };
    static constexpr const char k_appname[] = { "appname" };
    static constexpr const char k_exe[] = { "exe" };
    static constexpr const char k_StartDir[] = { "StartDir" };
    static constexpr const char k_icon[] = { "icon" };
    static constexpr const char k_ShortcutPath[] = { "ShortcutPath" };
    static constexpr const char k_LaunchOptions[] = { "LaunchOptions" };
    static constexpr const char k_IsHidden[] = { "IsHidden" };
    static constexpr const char k_AllowDesktopConfig[] = { "AllowDesktopConfig" };
    static constexpr const char k_AllowOverlay[] = { "AllowOverlay" };
    static constexpr const char k_openvr[] = { "openvr" };
    static constexpr const char k_Devkit[] = { "Devkit" };
    static constexpr const char k_DevkitGameID[] = { "DevkitGameID" };
    static constexpr const char k_DevkitOverrideAppID[] = { "DevkitOverrideAppID" };
    static constexpr const char k_LastPlayTime[] = { "LastPlayTime" };
    static constexpr const char k_tags[] = { "tags" };

    template<const char* const keyname, typename type, const uint8_t const _type_id>
    struct VDFKeyPair
    {
        VDFKeyPair() {}
        explicit VDFKeyPair(type _value) : value(_value) {}
        static constexpr uint8_t _TID = _type_id;
        static constexpr const char* const _KEY = keyname;
        const uint8_t type_id = _TID;
        const char* const key = _KEY;
        type value;
    };

    static constexpr uint32_t idx_begin_ = 0x30;
    struct ShortcutTags
    {
        const char idx[2] = "0";
        std::vector<const char*> tags;
        const uint16_t end_marker = 0x0808;
    };

    struct Shortcut
    {
        const char idx[2] = "0";
        VDFKeyPair<k_appId, uint32_t, 0x02> appId{ 0x000000 }; // TODO ???
        VDFKeyPair<k_appname, const char*, 0x01> appName{ "" };
        VDFKeyPair<k_exe, const char*, 0x01> exe{ "\"\"" }; // Qouted
        VDFKeyPair<k_StartDir, const char*, 0x01> StartDir{ "\"\"" }; // Qouted
        VDFKeyPair<k_icon, const char*, 0x01> icon{ "" }; // Qouted or empty
        VDFKeyPair<k_ShortcutPath, const char*, 0x01> ShortcutPath{ "" }; // Qouted or empty?
        VDFKeyPair<k_LaunchOptions, const char*, 0x01> LaunchOptions{ "" }; // UNQOUTED or empty
        VDFKeyPair<k_IsHidden, uint32_t, 0x02> IsHidden{ 0 };
        VDFKeyPair<k_AllowDesktopConfig, uint32_t, 0x02> AllowDesktopConfig{ 1 }; 
        VDFKeyPair<k_AllowOverlay, uint32_t, 0x02> AllowOverlay{ 1 }; 
        VDFKeyPair<k_openvr, uint32_t, 0x02> openvr{ 0 };
        VDFKeyPair<k_Devkit, uint32_t, 0x02> Devkit{ 0 };
        VDFKeyPair<k_DevkitGameID, const char*, 0x01> DevkitGameID{ "" };
        VDFKeyPair<k_DevkitOverrideAppID, uint32_t, 0x02> DevkitOverrideAppID{ 0 }; // 
        VDFKeyPair<k_LastPlayTime, uint32_t, 0x02> LastPlayTime{ 0 }; // 
        VDFKeyPair<k_tags, ShortcutTags, 0x00> tags{ }; 
    };

    struct VDFFile
    {
        const uint8_t first_byte = 0x00;
        const char* const identifier = "shortcuts";
        std::vector<Shortcut> shortcuts; // only use data...
        const uint16_t end_marker = 0x0808;
    };


    class Parser
    {
    public:
        static inline QList<QVariantMap> parseShortcuts(std::filesystem::path path)
        {

            VDFFile vdffile;
            vdffile.shortcuts.emplace_back();
            vdffile.shortcuts[0].tags.value.tags.emplace_back();

            QList<QVariantMap> res;

            QFile shortcuts_file(QString::fromStdWString(path.wstring()));
            if (!shortcuts_file.open(QFile::ReadWrite))
            {
                // TODO: try to create file...
                return res;
            }
            //const QByteArray content = shortcuts_file.readAll();
            //const QByteArray header = QByteArray(content.data(), 11);
            //if (file_header.compare(header) != 0)
            //{
            //    // TODO: invalid header
            //}

            shortcuts_file.close();

            return res;
        }
    };
}

//AppName = app.Name,
//Exe = exePath,
//StartDir = exeDir,
//LaunchOptions = app.Aumid,
//AllowDesktopConfig = 1,
//AllowOverlay = 1,
//Icon = app.Icon,
//Index = shortcuts.Length,
//IsHidden = 0,
//OpenVR = 0,
//ShortcutPath = "",
//Tags = tags,
//Devkit = 0,
//DevkitGameID = "",
//LastPlayTime = (int)DateTimeOffset.UtcNow.ToUnixTimeSeconds(),