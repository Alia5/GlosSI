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
#include <QJsonDocument>

#ifdef _WIN32
#include <Windows.h>
#include <VersionHelpers.h>
#include <collection.h>
#include <appmodel.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <windows.h>
#include <appxpackaging.h>
#pragma comment(lib, "Shlwapi.lib")
using namespace Windows::Management::Deployment;
using namespace Windows::Foundation::Collections;

#include <QGuiApplication>

#ifdef _WIN32
#include <WinReg/WinReg.hpp>
#endif

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

    std::for_each(entries.begin(), entries.end(), [this](const auto& name)
        {
            auto path = config_path_;
            path /= config_dir_name_.toStdString();
            path /= name.toStdString();
            QFile file(path);
            if (!file.open(QIODevice::Text | QIODevice::ReadOnly))
            {
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
            json["hideDevices"] = filejson["devices"]["hideDevices"];
            json["windowMode"] = filejson["window"]["windowMode"];
            json["maxFps"] = filejson["window"]["maxFps"];
            json["scale"] = filejson["window"]["scale"];

            json["name"] = QString(name).replace(QRegularExpression("\.json"), "");

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
    for (auto& steam_shortcut : shortcuts_vdf_.shortcuts)
    {
        if (map["name"].toString() == QString::fromStdString(steam_shortcut.appName.value))
        {
            if (QString::fromStdString(steam_shortcut.exe.value).toLower().contains("glossitarget.exe"))
            {
                return true;
            }
        }
    }

    return false;
}

bool UIModel::addToSteam(QVariant shortcut)
{
    QDir appDir = QDir::current();
    const auto map = shortcut.toMap();
    const auto name = map["name"].toString();
    const auto maybeLaunchPath = map["launchPath"].toString();
    const auto launch = map["launch"].toBool();

    VDFParser::Shortcut vdfshortcut;
    vdfshortcut.idx = shortcuts_vdf_.shortcuts.size();
    vdfshortcut.appName.value = name.toStdString();
    vdfshortcut.exe.value = ("\"" + appDir.absolutePath() + "/GlosSITarget.exe" + "\"").toStdString();
    vdfshortcut.StartDir.value = (
        launch && !maybeLaunchPath.isEmpty()
            ? (std::string("\"") + std::filesystem::path(maybeLaunchPath.toStdString()).parent_path().string() + "\"")
            : ("\"" + appDir.absolutePath() + "\"").toStdString()
        );
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
    if (maybeIcon.isEmpty())
    {
        if (launch && !maybeLaunchPath.isEmpty())
            vdfshortcut.icon.value = maybeLaunchPath.toStdString();
    } else {
        vdfshortcut.icon.value = maybeIcon.toStdString();
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
    scuts.erase(std::remove_if(scuts.begin(), scuts.end(), [&name](const auto& shortcut)
    {
            return shortcut.appName.value == name.toStdString();
    }), scuts.end());
    for (int i = 0; i < scuts.size(); i++)
    {
        if (scuts[i].idx != i)
        {
            scuts[i].idx = i;
        }
    }
    const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() + getSteamUserId() + shortcutsfile_.toStdWString();
    return VDFParser::Parser::writeShortcuts(config_path, shortcuts_vdf_);
}

#ifdef _WIN32
QVariantList UIModel::uwpApps()
{
    // TODO really should do this async, and notify gui when complete...
    if (!IsWindows10OrGreater())
    {
        return QVariantList();
    }

    std::vector<std::wstring> logoNames{
        L"Square150x150Logo",
        L"Square310x310Logo",
        L"Square44x44Logo",
        L"Square71x71Logo",
        L"Square70x70Logo",
        L"Logo",
        L"SmallLogo",
        L"Square30x30Logo",
    };

    QVariantList pairs;
    // is it considered stealing when you take code that was pull-requested by someone else into your own repo?
    // Anyway... Stolen from: https://github.com/Thracky/GloSC/commit/3cd92e058498e3ab9d73ced140bbd7e490f639a7
    // https://github.com/Alia5/GloSC/commit/3cd92e058498e3ab9d73ced140bbd7e490f639a7


        // TODO: only return apps for current user.
        // TODO: I have no clue how this WinRT shit works; HELP MEH!

    PackageManager^ packageManager = ref new PackageManager();
    IIterable<Windows::ApplicationModel::Package^>^ packages = packageManager->FindPackages();

    int packageCount = 0;
    // Only way to get the count of packages is to iterate through the whole collection first
    std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
        [&](Windows::ApplicationModel::Package^ package)
        {
            packageCount += 1;
        });

    int currPackage = 0;
    // Iterate through all the packages
    std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
        [&](Windows::ApplicationModel::Package^ package)
        {
            QGuiApplication::processEvents();
            HRESULT hr = S_OK;
            IStream* inputStream = NULL;
            UINT32 pathLen = 0;
            IAppxManifestReader* manifestReader = NULL;
            IAppxFactory* appxFactory = NULL;
            LPWSTR appId = NULL;
            LPWSTR manifestAppName = NULL;
            LPWSTR iconName = NULL;

            // Get the package path on disk so we can load the manifest XML and get the PRAID
            GetPackagePathByFullName(package->Id->FullName->Data(), &pathLen, NULL);

            if (pathLen > 0) {

                // Length of the path + "\\AppxManifest.xml" that we'll be appending
                UINT32 manifestLen = pathLen + 20;
                PWSTR pathBuf = (PWSTR)malloc(manifestLen * sizeof(wchar_t));

                GetPackagePathByFullName(package->Id->FullName->Data(), &pathLen, pathBuf);
                PWSTR manifest_xml = L"\\AppxManifest.xml";

                hr = StringCchCatW(pathBuf, manifestLen, manifest_xml);

                // Let's ignore a bunch of built in apps and such
                if (wcsstr(pathBuf, L"SystemApps")) {
                    hr = E_FAIL;
                }
                else if (wcsstr(pathBuf, L".NET.Native."))
                    hr = E_FAIL;
                else if (wcsstr(pathBuf, L".VCLibs."))
                    hr = E_FAIL;
                else if (wcsstr(pathBuf, L"Microsoft.UI"))
                    hr = E_FAIL;
                else if (wcsstr(pathBuf, L"Microsoft.Advertising"))
                    hr = E_FAIL;
                else if (wcsstr(pathBuf, L"Microsoft.Services.Store"))
                    hr = E_FAIL;

                BOOL hasCurrent = FALSE;

                // Open the manifest XML
                if (SUCCEEDED(hr)) {

                    hr = SHCreateStreamOnFileEx(
                        pathBuf,
                        STGM_READ | STGM_SHARE_EXCLUSIVE,
                        0, // default file attributes
                        FALSE, // do not create new file
                        NULL, // no template
                        &inputStream);
                }
                if (SUCCEEDED(hr)) {

                    hr = CoCreateInstance(
                        __uuidof(AppxFactory),
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        __uuidof(IAppxFactory),
                        (LPVOID*)(&appxFactory));
                }
                if (SUCCEEDED(hr)) {
                    hr = appxFactory->CreateManifestReader(inputStream, &manifestReader);
                }

                // Grab application ID (PRAID) and DisplayName from the XML
                if (SUCCEEDED(hr)) {
                    IAppxManifestApplicationsEnumerator* applications = NULL;
                    manifestReader->GetApplications(&applications);
                    if (SUCCEEDED(hr)) {
                        hr = applications->GetHasCurrent(&hasCurrent);
                        if (hasCurrent) {
                            IAppxManifestApplication* application = NULL;
                            hr = applications->GetCurrent(&application);
                            if (SUCCEEDED(hr)) {
                                application->GetStringValue(L"Id", &appId);
                                application->GetStringValue(L"DisplayName", &manifestAppName);
                                for (auto& logoNameStr : logoNames)
                                {
                                    application->GetStringValue(logoNameStr.c_str(), &iconName);
                                    if (!std::wstring(iconName).empty())
                                    {
                                        break;
                                    }
                                }
                                application->Release();
                            }
                        }
                        else {
                            hr = S_FALSE;
                        }
                        applications->Release();
                    }
                    manifestReader->Release();
                    inputStream->Release();

                }

                if (SUCCEEDED(hr)) {
                    PWSTR appNameBuf;
                    QString AppUMId = QString::fromWCharArray(package->Id->FamilyName->Data());
                    QString AppName;
                    QString Path = QString::fromWCharArray(package->EffectivePath->Data());
                    // QString thumbToken = QString::fromWCharArray(package->GetThumbnailToken()->Data());
                    if (manifestAppName != NULL)
                    {
                        // If the display name is an indirect string, we'll try and load it using SHLoadIndirectString
                        if (wcsstr(manifestAppName, L"ms-resource:"))
                        {
                            PWSTR res_name = wcsdup(&manifestAppName[12]);
                            appNameBuf = (PWSTR)malloc(1026);
                            LPCWSTR resource_str = L"@{";
                            std::wstring reslookup = std::wstring(resource_str) + package->Id->FullName->Data() + L"?ms-resource://" + package->Id->Name->Data() + L"/resources/" + res_name + L"}";
                            PCWSTR res_str = reslookup.c_str();
                            hr = SHLoadIndirectString(res_str, appNameBuf, 512, NULL);
                            // Try several resource paths
                            if (!SUCCEEDED(hr)) {
                                std::wstring reslookup = std::wstring(resource_str) + package->Id->FullName->Data() + L"?ms-resource://" + package->Id->Name->Data() + L"/Resources/" + res_name + L"}";
                                PCWSTR res_str = reslookup.c_str();
                                hr = SHLoadIndirectString(res_str, appNameBuf, 512, NULL);
                                // If the third one doesn't work, we give up and use the package name from PackageManager
                                if (!SUCCEEDED(hr)) {
                                    std::wstring reslookup = std::wstring(resource_str) + package->Id->FullName->Data() + L"?ms-resource://" + package->Id->Name->Data() + L"/" + res_name + L"}";
                                    PCWSTR res_str = reslookup.c_str();
                                    hr = SHLoadIndirectString(res_str, appNameBuf, 512, NULL);
                                }
                            }

                            if (!SUCCEEDED(hr))
                                AppName = QString::fromWCharArray(package->DisplayName->Data());
                            else
                                AppName = QString::fromWCharArray(appNameBuf);
                            free(appNameBuf);
                        }
                        else
                        {
                            appNameBuf = manifestAppName;
                            AppName = QString::fromWCharArray(appNameBuf);
                        }

                    }
                    else {
                        AppName = QString::fromWCharArray(package->DisplayName->Data());
                    }

                    QString PRAID = QString::fromWCharArray(appId);
                    CoTaskMemFree(appId);
                    if (!PRAID.isEmpty()) {
                        AppUMId = AppUMId.append("!");
                        AppUMId = AppUMId.append(PRAID);
                    }
                    QVariantMap uwpPair;
                    uwpPair.insert("AppName", AppName);
                    uwpPair.insert("AppUMId", AppUMId);
                    uwpPair.insert("Path", Path);

                    QString icoFName = Path + "/" + QString::fromWCharArray(iconName);
                    std::filesystem::path icoPath(icoFName.toStdString());

                    std::vector<QString> possibleextensions = { ".scale-100", ".scale-125", ".scale-150", ".scale-200" };
                    if (!std::filesystem::exists(icoPath))
                    {
                        for (const auto& ext : possibleextensions)
                        {
                            QString maybeFname = QString(icoFName).replace(".png", ext + ".png");
                            std::filesystem::path maybePath(maybeFname.toStdString());
                            if (std::filesystem::exists(maybePath))
                            {
                                icoPath = maybePath;
                                break;
                            }
                        }
                    }

                    uwpPair.insert("IconPath", QString::fromStdString(icoPath.string()));

                    free(pathBuf);

                    pairs.push_back(uwpPair);
                }
            }
            currPackage += 1;
        });
    return pairs;
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
    path /= config_dir_name_.toStdString();
    path /= (name + ".json").toStdString();
    QFile file(path);
    if (!file.open(QIODevice::Text | QIODevice::ReadWrite))
    {
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
    winreg::RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam" };
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
    winreg::RegKey key{ HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess" };
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