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
#include "UIModel.h"

#include <QDir>
#include <QGuiApplication>
#include <QJsonDocument>

#include <WinReg/WinReg.hpp>

#ifdef _WIN32
#include "UWPFetch.h"
#endif

UIModel::UIModel() : QObject(nullptr)
{
    auto path = std::filesystem::temp_directory_path()
                    .parent_path()
                    .parent_path()
                    .parent_path();

    path /= "Roaming";
    path /= "GlosSI";
    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    config_path_ = path;
    config_dir_name_ = (path /= "Targets").string().data();

    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    parseShortcutVDF();
    readConfigs();
}

void UIModel::readConfigs()
{
    QDir dir(config_dir_name_);
    auto entries = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    entries.removeIf([](const auto& entry) {
        return !entry.endsWith(".json");
    });

    std::for_each(entries.begin(), entries.end(), [this](const auto& name) {
        auto path = config_path_;
        path /= config_dir_name_.toStdString();
        path /= name.toStdWString();
        QFile file(path);
        if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            // meh
            return;
        }
        const auto data = file.readAll();
        file.close();
        const auto jsondoc = QJsonDocument::fromJson(data);
        const auto filejson = jsondoc.object();

        QJsonObject json;
        json["version"] = filejson["version"];
        json["icon"] = filejson["icon"];
        json["launch"] = filejson["launch"]["launch"];
        json["launchPath"] = filejson["launch"]["launchPath"];
        json["launchAppArgs"] = filejson["launch"]["launchAppArgs"];
        json["closeOnExit"] = filejson["launch"]["closeOnExit"];
        json["waitForChildProcs"] = filejson["launch"]["waitForChildProcs"];
        json["hideDevices"] = filejson["devices"]["hideDevices"];
        json["windowMode"] = filejson["window"]["windowMode"];
        json["maxFps"] = filejson["window"]["maxFps"];
        json["scale"] = filejson["window"]["scale"];

        json["name"] = QString(name).replace(QRegularExpression("\\.json"), "");

        targets_.append(json.toVariantMap());
    });

    emit targetListChanged();
}

QVariantList UIModel::getTargetList() const
{
    return targets_;
}

void UIModel::addTarget(QVariant shortcut)
{
    const auto map = shortcut.toMap();
    const auto json = QJsonObject::fromVariantMap(map);
    writeTarget(json, map["name"].toString());
    targets_.append(QJsonDocument(json).toVariant());
    emit targetListChanged();
}

void UIModel::updateTarget(int index, QVariant shortcut)
{
    const auto map = shortcut.toMap();
    const auto json = QJsonObject::fromVariantMap(map);

    auto oldName = targets_[index].toMap()["name"].toString() + ".json";
    auto path = config_path_;
    path /= config_dir_name_.toStdString();
    path /= (oldName).toStdString();
    std::filesystem::remove(path);

    writeTarget(json, map["name"].toString());

    targets_.replace(index, QJsonDocument(json).toVariant());
    emit targetListChanged();
}

void UIModel::deleteTarget(int index)
{
    auto oldName = targets_[index].toMap()["name"].toString() + ".json";
    auto path = config_path_;
    path /= config_dir_name_.toStdString();
    path /= (oldName).toStdString();
    std::filesystem::remove(path);
    targets_.remove(index);
    emit targetListChanged();
}

bool UIModel::isInSteam(QVariant shortcut)
{
    const auto map = shortcut.toMap();
    for (auto& steam_shortcut : shortcuts_vdf_.shortcuts) {
        if (map["name"].toString() == QString::fromStdString(steam_shortcut.appName.value)) {
            if (QString::fromStdString(steam_shortcut.exe.value).toLower().contains("glossitarget.exe")) {
                return true;
            }
        }
    }

    return false;
}

bool UIModel::addToSteam(QVariant shortcut)
{
    QDir appDir = QGuiApplication::applicationDirPath();
    const auto map = shortcut.toMap();
    const auto name = map["name"].toString();
    const auto maybeLaunchPath = map["launchPath"].toString();
    const auto launch = map["launch"].toBool();

    VDFParser::Shortcut vdfshortcut;
    vdfshortcut.idx = shortcuts_vdf_.shortcuts.size();
    vdfshortcut.appName.value = name.toStdString();
    vdfshortcut.exe.value = ("\"" + appDir.absolutePath() + "/GlosSITarget.exe" + "\"").toStdString();
    vdfshortcut.StartDir.value = (launch && !maybeLaunchPath.isEmpty()
                                      ? (std::string("\"") + std::filesystem::path(maybeLaunchPath.toStdString()).parent_path().string() + "\"")
                                      : ("\"" + appDir.absolutePath() + "\"").toStdString());
    vdfshortcut.appId.value = VDFParser::Parser::calculateAppId(vdfshortcut);
    // ShortcutPath; default
    vdfshortcut.LaunchOptions.value = (name + ".json").toStdString();
    // IsHidden; default
    // AllowDesktopConfig; default
    // AllowOverlay; default
    // openvr; default
    // Devkit; default
    // DevkitGameID; default
    // DevkitOverrideAppID; default
    // LastPlayTime; default
    auto maybeIcon = map["icon"].toString();
    if (maybeIcon.isEmpty()) {
        if (launch && !maybeLaunchPath.isEmpty())
            vdfshortcut.icon.value =
                "\"" + (
                    is_windows_
                        ? QString(maybeLaunchPath).replace(QRegularExpression("\\/"), "\\").toStdString()
                        : maybeLaunchPath.toStdString()
                    ) + "\"";
    }
    else {
        vdfshortcut.icon.value =
            "\"" + (
                is_windows_
                    ? QString(maybeIcon).replace(QRegularExpression("\\/"), "\\").toStdString()
                    : maybeIcon.toStdString()
                ) + "\"";
    }
    // Add installed locally and GlosSI tag
    VDFParser::ShortcutTag locallyTag;
    locallyTag.idx = 0;
    locallyTag.value = "Installed locally";
    vdfshortcut.tags.value.push_back(locallyTag);

    VDFParser::ShortcutTag glossitag;
    glossitag.idx = 1;
    glossitag.value = "GlosSI";
    vdfshortcut.tags.value.push_back(glossitag);

    shortcuts_vdf_.shortcuts.push_back(vdfshortcut);

    const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() + getSteamUserId() + shortcutsfile_.toStdWString();
    return VDFParser::Parser::writeShortcuts(config_path, shortcuts_vdf_);
}
bool UIModel::removeFromSteam(const QString& name)
{
    auto& scuts = shortcuts_vdf_.shortcuts;
    scuts.erase(std::remove_if(scuts.begin(), scuts.end(), [&name](const auto& shortcut) {
                    return shortcut.appName.value == name.toStdString();
                }),
                scuts.end());
    for (int i = 0; i < scuts.size(); i++) {
        if (scuts[i].idx != i) {
            scuts[i].idx = i;
        }
    }
    const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() + getSteamUserId() + shortcutsfile_.toStdWString();
    return VDFParser::Parser::writeShortcuts(config_path, shortcuts_vdf_);
}

#ifdef _WIN32
QVariantList UIModel::uwpApps()
{
    return UWPFetch::UWPAppList();
}
#endif

bool UIModel::getIsWindows() const
{
    return is_windows_;
}

bool UIModel::hasAcrylicEffect() const
{
    return has_acrylic_affect_;
}

void UIModel::setAcrylicEffect(bool has_acrylic_affect)
{
    has_acrylic_affect_ = has_acrylic_affect;
    emit acrylicChanged();
}

void UIModel::writeTarget(const QJsonObject& json, const QString& name)
{
    auto path = config_path_;
    path /= config_dir_name_.toStdWString();
    path /= (name + ".json").toStdWString();
    QFile file(path);
    if (!file.open(QIODevice::Text | QIODevice::ReadWrite)) {
        // meh
        return;
    }
    QJsonObject fileJson;
    fileJson["version"] = json["version"];
    fileJson["icon"] = json["icon"];

    QJsonObject launchObject;
    launchObject["launch"] = json["launch"];
    launchObject["launchPath"] = json["launchPath"];
    launchObject["launchAppArgs"] = json["launchAppArgs"];
    launchObject["closeOnExit"] = json["closeOnExit"];
    launchObject["waitForChildProcs"] = json["waitForChildProcs"];
    fileJson["launch"] = launchObject;

    QJsonObject devicesObject;
    devicesObject["hideDevices"] = json["hideDevices"];
    fileJson["devices"] = devicesObject;

    QJsonObject windowObject;
    windowObject["windowMode"] = json["windowMode"];
    windowObject["maxFps"] = json["maxFps"];
    windowObject["scale"] = json["scale"];
    fileJson["window"] = windowObject;

    auto wtf = QString(QJsonDocument(fileJson).toJson(QJsonDocument::Indented)).toStdString();
    file.write(wtf.data());
    file.close();
}

std::filesystem::path UIModel::getSteamPath() const
{
#ifdef _WIN32
    // TODO: check if keys/value exist
    // steam should always be open and have written reg values...
    winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam"};
    const auto res = key.GetStringValue(L"SteamPath");
    return res;
#else
    return L""; // TODO LINUX
#endif
}

std::wstring UIModel::getSteamUserId() const
{
#ifdef _WIN32
    // TODO: check if keys/value exist
    // steam should always be open and have written reg values...
    winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess"};
    const auto res = std::to_wstring(key.GetDwordValue(L"ActiveUser"));
    return res;
#else
    return L""; // TODO LINUX
#endif
}

void UIModel::parseShortcutVDF()
{
    const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() + getSteamUserId() + shortcutsfile_.toStdWString();
    shortcuts_vdf_ = VDFParser::Parser::parseShortcuts(config_path);
}
