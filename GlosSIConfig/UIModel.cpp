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
#include "UIModel.h"

#include <QDir>
#include <QFont>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMetaType>


#include <WinReg/WinReg.hpp>

#include <ranges>

#ifdef _WIN32
#include "UWPFetch.h"
#include <Windows.h>
#endif

#include "ExeImageProvider.h"
#include "../version.hpp"

#include "../common/UnhookUtil.h"

#include "../common/util.h"

UIModel::UIModel() : QObject(nullptr)
{
    auto path = util::path::getDataDirPath();

    qDebug() << "Version: " << getVersionString();

    config_path_ = path;
    config_dir_name_ = QString::fromStdWString((path /= "Targets").wstring());

    if (!std::filesystem::exists(path))
        std::filesystem::create_directories(path);

    auto defaultConf = getDefaultConf();
    saveDefaultConf(defaultConf);

    parseShortcutVDF();
    readTargetConfigs();
    updateCheck();
    readUnhookBytes();

    auto font = QGuiApplication::font();
    font.setPointSize(11);
    font.setFamily("Roboto");
    QGuiApplication::setFont(font);

    std::ofstream{getSteamPath() / ".cef-enable-remote-debugging"};


}

void UIModel::readTargetConfigs()
{
    QDir dir(config_dir_name_);
    auto entries = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    entries.removeIf([](const auto& entry) { return !entry.endsWith(".json"); });

    std::for_each(entries.begin(), entries.end(), [this](const auto& name) {
        auto path = config_path_;
        path /= config_dir_name_.toStdWString();
        path /= name.toStdWString();
        QFile file(path);
        if (!file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            // meh
            return;
        }
        const auto data = file.readAll();
        file.close();
        const auto jsondoc = QJsonDocument::fromJson(data);
        auto filejson = jsondoc.object();

        filejson["name"] = filejson.contains("name") ? filejson["name"].toString()
                                                     : QString(name).replace(QRegularExpression("\\.json"), "");

        targets_.append(filejson.toVariantMap());
    });

    emit targetListChanged();
}

QVariantList UIModel::getTargetList() const { return targets_; }

void UIModel::addTarget(QVariant shortcut)
{
    const auto map = shortcut.toMap();
    const auto json = QJsonObject::fromVariantMap(map);
    writeTarget(json, map["name"].toString());
    targets_.append(QJsonDocument(json).toVariant());
    emit targetListChanged();
}

bool UIModel::updateTarget(int index, QVariant shortcut)
{
    const auto map = shortcut.toMap();
    const auto json = QJsonObject::fromVariantMap(map);

    const auto was_in_steam_ = isInSteam(shortcut);

    auto oldSteamName = targets_[index].toMap()["name"].toString();
    auto oldName =
        targets_[index].toMap()["name"].toString().replace(QRegularExpression("[\\\\/:*?\"<>|]"), "") + ".json";
    auto oldPath = config_path_;
    oldPath /= config_dir_name_.toStdString();
    oldPath /= (oldName).toStdString();
    std::filesystem::remove(oldPath);

    writeTarget(json, map["name"].toString());

    targets_.replace(index, QJsonDocument(json).toVariant());
    emit targetListChanged();

    auto path = config_path_;
    path /= config_dir_name_.toStdString();
    path /= (map["name"].toString()).toStdString();

    if (was_in_steam_) {
        if (removeFromSteam(oldSteamName, QString::fromStdWString(path.wstring()))) {
            if (!addToSteam(shortcut, QString::fromStdWString(path.wstring()))) {
                qDebug() << "Couldn't add shortcut \"" << (map["name"].toString()) << "\" to Steam when updating";
                return false;
            }
            return true;
        }
        qDebug() << "Couldn't remove shortcut \"" << oldName << "\" from Steam when updating";
        return false;
    } else {
        return true;
    }
}

void UIModel::deleteTarget(int index)
{
    auto oldName =
        targets_[index].toMap()["name"].toString().replace(QRegularExpression("[\\\\/:*?\"<>|]"), "") + ".json";
    auto path = config_path_;
    path /= config_dir_name_.toStdString();
    path /= (oldName).toStdString();
    std::filesystem::remove(path);
    targets_.remove(index);
    emit targetListChanged();
}

bool UIModel::isInSteam(QVariant shortcut) const
{
    const auto map = shortcut.toMap();
    for (auto& steam_shortcut : shortcuts_vdf_) {
        if (
            map["name"].toString() == QString::fromStdString(steam_shortcut.appname) ||
            map["oldName"].toString() == QString::fromStdString(steam_shortcut.appname)) {
            if (QString::fromStdString(steam_shortcut.exe).toLower().contains("glossitarget.exe")) {
                return true;
            }
        }
    }

    return false;
}

uint32_t UIModel::getAppId(QVariant shortcut)
{
    if (!isInSteam(shortcut)) {
        return 0;
    }
    const auto map = shortcut.toMap();
    for (auto& steam_shortcut : shortcuts_vdf_) {
        if (map["name"].toString() == QString::fromStdString(steam_shortcut.appname)) {
            if (QString::fromStdString(steam_shortcut.exe).toLower().contains("glossitarget.exe")) {
                if (steam_shortcut.appid == 0) {
                    parseShortcutVDF();
                    return getAppId(shortcut);
                }
                return steam_shortcut.appid;
            }
        }
    }
    return 0;
}

Q_INVOKABLE QString UIModel::getGameId(QVariant shortcut) {

	/*
    * enum SteamLaunchableType
        {
            App = 0,
            GameMod = 1,
            Shortcut = 2,
            P2P = 3
        }
    */
    uint64_t gameId = (((uint64_t)getAppId(shortcut) << 32) | ((uint32_t)2 << 24) | 0);
    return QVariant(gameId).toString();
}

bool UIModel::addToSteam(QVariant shortcut, const QString& shortcutspath, bool from_cmd)
{
    QDir appDir = QGuiApplication::applicationDirPath();
    const auto map = shortcut.toMap();
    const auto name = map["name"].toString();
    const auto maybeLaunchPath = map["launchPath"].toString();
    const auto launch = map["launch"].toBool();

    VDFParser::Shortcut vdfshortcut;
    vdfshortcut.appname = name.toStdString();
    vdfshortcut.exe = ("\"" + appDir.absolutePath() + "/GlosSITarget.exe" + "\"").toStdString();
    vdfshortcut.StartDir =
        (launch && !maybeLaunchPath.isEmpty()
             ? (std::string("\"") + std::filesystem::path(maybeLaunchPath.toStdString()).parent_path().string() + "\"")
             : ("\"" + appDir.absolutePath() + "\"").toStdString());
    // ShortcutPath; default
    vdfshortcut.LaunchOptions =
        (QString(name).replace(QRegularExpression("[\\\\/:*?\"<>|]"), "") + ".json").toStdString();
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
            vdfshortcut.icon =
                "\"" +
                (is_windows_ ? QString(maybeLaunchPath).replace(QRegularExpression("\\/"), "\\").toStdString()
                             : maybeLaunchPath.toStdString()) +
                "\"";
    }
    else {
        vdfshortcut.icon = "\"" +
                           (is_windows_ ? QString(maybeIcon).replace(QRegularExpression("\\/"), "\\").toStdString()
                                        : maybeIcon.toStdString()) +
                           "\"";
    }
    // Add installed locally and GlosSI tag
    vdfshortcut.tags.push_back("Installed locally");
    vdfshortcut.tags.push_back("GlosSI");

    shortcuts_vdf_.push_back(vdfshortcut);

    return writeShortcutsVDF(L"add", name.toStdWString(), shortcutspath.toStdWString(), from_cmd);
}
bool UIModel::addToSteam(const QString& name, const QString& shortcutspath, bool from_cmd)
{
    qDebug() << "trying to add " << name << " to steam";
    const auto target = std::find_if(targets_.begin(), targets_.end(), [&name](const auto& target) {
        const auto map = target.toMap();
        const auto target_name = map["name"].toString().replace(QRegularExpression("[\\\\/:*?\"<>|]"), "");
        return name == target_name;
    });
    if (target != targets_.end()) {
        return addToSteam(*target, shortcutspath, from_cmd);
    }
    qDebug() << name << " not found!";
    return false;
}
bool UIModel::removeFromSteam(const QString& name, const QString& shortcutspath, bool from_cmd)
{
    qDebug() << "trying to remove " << name << " from steam";
    shortcuts_vdf_.erase(
        std::ranges::remove_if(shortcuts_vdf_,
                               [&name](const auto& shortcut) { return shortcut.appname == name.toStdString(); })
            .begin(),
        shortcuts_vdf_.end());
    return writeShortcutsVDF(L"remove", name.toStdWString(), shortcutspath.toStdWString(), from_cmd);
}

QVariantMap UIModel::manualProps(QVariant shortcut)
{
    QDir appDir = QGuiApplication::applicationDirPath();
    const auto map = shortcut.toMap();
    const auto name = map["name"].toString().replace(QRegularExpression("[\\\\/:*?\"<>|]"), "");
    const auto maybeLaunchPath = map["launchPath"].toString();
    const auto launch = map["launch"].toBool();

    QVariantMap res;
    res.insert("name", name);
    res.insert("config", name + ".json");
    res.insert("launch", ("\"" + appDir.absolutePath() + "/GlosSITarget.exe" + "\""));
    res.insert(
        "launchDir",
        (launch && !maybeLaunchPath.isEmpty()
             ? (QString("\"") +
                QString::fromStdString(std::filesystem::path(maybeLaunchPath.toStdString()).parent_path().string()) +
                "\"")
             : ("\"" + appDir.absolutePath() + "\"")));
    return res;
}

void UIModel::enableSteamInputXboxSupport()
{
    if (foundSteam()) {
        const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() +
                                                  getSteamUserId() + user_config_file_.toStdWString();
        if (!std::filesystem::exists(config_path)) {
            qDebug() << "localconfig.vdf does not exist.";
        }
        QFile file(config_path);
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            QTextStream in(&file);
            QStringList lines;
            QString line = in.readLine();
            // simple approach is enough...
            while (!in.atEnd()) {
                if (line.contains("SteamController_XBoxSupport")) {
                    if (line.contains("1")) {
                        qDebug() << "\"SteamController_XBoxSupport\" is already enabled! aborting write...";
                        file.close();
                        return;
                    }
                    qDebug() << "found \"SteamController_XBoxSupport\" line, replacing value...";
                    line.replace("0", "1");
                }
                lines.push_back(line);
                line = in.readLine();
            }
            file.close();
            QFile updatedFile(config_path);
            if (updatedFile.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
                qDebug() << "writing localconfig.vdf...";
                QTextStream out(&updatedFile);
                for (const auto& l : lines) {
                    out << l << "\n";
                }
            }
            updatedFile.close();
        }
    }
}

bool UIModel::restartSteam(const QString& steamURL)
{
    const auto path = getSteamPath();
    if (QProcess::execute("taskkill.exe", {"/im", steam_executable_name_, "/f"}) != 0) {
        return false;
    }
    if (steamURL.isEmpty()) {
        QProcess::startDetached(QString::fromStdWString(path) + "/" + steam_executable_name_);
    }
    else {
        system((QString::fromLatin1("start ") + steamURL).toStdString().c_str());
    }
    return true;
}

void UIModel::updateCheck()
{
    auto manager = new QNetworkAccessManager();
    QNetworkRequest request;
    QNetworkReply* reply = NULL;

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2);
    request.setSslConfiguration(config);
    request.setUrl(QUrl("https://glossi.1-3-3-7.dev/api/availFiles"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    reply = manager->get(request);
    // connect(
    //     manager, &QNetworkAccessManager::finished, this, [](QNetworkReply* rep) {
    //         qDebug() << rep->readAll();
    //     });
    connect(manager, &QNetworkAccessManager::finished, this, &UIModel::onAvailFilesResponse);
}

QVariantMap UIModel::getDefaultConf() const
{
    auto path = util::path::getDataDirPath();

    path /= "default.json";
    
    QJsonObject defaults = {
        {"icon", QJsonValue::Null},
        {"name", QJsonValue::Null},
        {"version", 1},
        {"extendedLogging", false},
        {"snapshotNotify", false},
        {"steamgridApiKey", QJsonValue::Null},
        {"steamPath",
         QJsonValue::fromVariant(QString::fromStdWString(getSteamPath(false).wstring()))},
        {"steamUserId",
         QJsonValue::fromVariant(QString::fromStdWString(getSteamUserId(false)))},
        {"globalModeGameId", ""},
        {"globalModeUseGamepadUI", true},
        {"minimizeSteamGamepadUI", true},
        {"controller", QJsonObject{{"maxControllers", -1}, {"emulateDS4", false}, {"allowDesktopConfig", false}}},
        {"devices",
         QJsonObject{
             {"hideDevices", true},
             {"realDeviceIds", false},
         }},
        {"launch",
         QJsonObject{
             {"closeOnExit", true},
             {"launch", false},
             {"launchAppArgs", QJsonValue::Null},
             {"launchPath", QJsonValue::Null},
             {"waitForChildProcs", true},
             {"launcherProcesses", QJsonArray{}},
             {"ignoreLauncher", true},
             {"killLauncher", false},
         }},
        {"window",
         QJsonObject{
             {"disableOverlay", false},
             {"hideAltTab", true},
             {"maxFps", QJsonValue::Null},
             {"scale", QJsonValue::Null},
             {"windowMode", false},
             {"disableGlosSIOverlay", false}
         }},
    };

    if (std::filesystem::exists(path)) {
        QFile file(QString::fromStdWString(path));
        if (file.open(QIODevice::ReadOnly)) {
            const auto data = file.readAll();
            file.close();
            auto json = QJsonDocument::fromJson(data).object();

            const auto applyDefaults = [](QJsonObject obj, const QJsonObject& defaults,
                                          auto applyDefaultsFn) -> QJsonObject {
                for (const auto& key : defaults.keys()) {
                    qDebug() << key << ": " << obj[key];
                    if ((obj[key].isUndefined() || obj[key].isNull()) && !defaults[key].isNull()) {
                        obj[key] = defaults.value(key);
                    }
                    if (obj.value(key).isObject()) {
                        obj[key] =
                            applyDefaultsFn(obj[key].toObject(), defaults.value(key).toObject(), applyDefaultsFn);
                    }
                }
                return obj;
            };
            json = applyDefaults(json, defaults, applyDefaults);
            return json.toVariantMap();
        }
    }

    saveDefaultConf(defaults.toVariantMap());
    return getDefaultConf();
}

void UIModel::saveDefaultConf(QVariantMap conf) const
{
    auto path = util::path::getDataDirPath();

    path /= "default.json";

    QFile file(path);
    if (!file.open(QIODevice::Text | QIODevice::ReadWrite)) {
        qDebug() << "Couldn't open file for writing: " << path;
        return;
    }

    file.write(QString(QJsonDocument::fromVariant(conf).toJson(QJsonDocument::Indented)).toStdString().data());
    file.close();
}

Q_INVOKABLE QVariant UIModel::globalModeShortcutConf() {
    for (auto& target : targets_) {
        const auto map = target.toMap();
        if (map["name"] == "GlosSI GlobalMode/Desktop") {
            return target;
        }
    }
    return QVariant();
}

Q_INVOKABLE bool UIModel::globalModeShortcutExists() {
    const auto map = globalModeShortcutConf().toMap();
    if (map["name"] == "GlosSI GlobalMode/Desktop") {
        return true;
    }
    return false;
}

Q_INVOKABLE uint32_t UIModel::globalModeShortcutAppId() {
    if (!globalModeShortcutExists()) {
        return 0;
    }
    return getAppId(globalModeShortcutConf());
}

Q_INVOKABLE QString UIModel::globalModeShortcutGameId() {
    if (!globalModeShortcutExists()) {
        return "";
    }
    return getGameId(globalModeShortcutConf());
}

#ifdef _WIN32
QVariantList UIModel::uwpApps() { return UWPFetch::UWPAppList(); }
#endif

QVariantList UIModel::egsGamesList() const
{
    wchar_t* program_data_path_str;
    std::filesystem::path path;
    if (SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_CREATE, NULL, &program_data_path_str) != S_OK) {
        qDebug() << "Couldn't get ProgramDataPath";
        return {{"InstallLocation", "Error"}};
    }
    path = std::filesystem::path(program_data_path_str);
    path /= egs_games_json_path_;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        const auto data = file.readAll();
        file.close();
        auto json = QJsonDocument::fromJson(data).object();
        if (json["InstallationList"].isArray()) {
            return json["InstallationList"].toVariant().toList();
        }
        qDebug() << "InstallationList does not exist!";
    }
    qDebug() << "Couldn't read EGS LauncherInstalled.dat " << path;
    return {{"InstallLocation", "Error"}};
}

void UIModel::loadSteamGridImages()
{
    std::filesystem::path path = QCoreApplication::applicationDirPath().toStdWString();
    path /= "steamgrid.exe";

    auto api_key = getDefaultConf().value("steamgridApiKey").toString();
    steamgrid_output_.clear();

    steamgrid_proc_.setProgram(path.string().c_str());
    steamgrid_proc_.setArguments({"-nonsteamonly", "--onlymissingartwork", "--steamgriddb", api_key});
    connect(&steamgrid_proc_, &QProcess::readyReadStandardOutput, this, &UIModel::onSteamGridReadReady);
        steamgrid_proc_.start();
    steamgrid_proc_.write("\n");
}

QString UIModel::getGridImagePath(QVariant shortcut)
{
    if (!foundSteam()) {
        return "";
    }
    const auto& app_id = getAppId(shortcut);
    if (app_id == 0) {
        return "";
    }

    const std::filesystem::path grid_dir =
        std::wstring(getSteamPath()) + user_data_path_.toStdWString() + getSteamUserId() + L"/config/grid";
    if (!std::filesystem::exists(grid_dir)) {
        return "";
    }
    const std::vector<std::string> extensions = {".png", ".jpg"};
    for (const auto& entry : std::filesystem::directory_iterator(grid_dir)) {
        if (entry.is_regular_file() &&
            std::ranges::find(extensions, entry.path().extension().string()) != extensions.end() &&
            entry.path().filename().string().find(std::to_string(app_id)) != std::string::npos) {
            return QString::fromStdString(entry.path().string());
        }
    }
    return "";
}

bool UIModel::writeShortcutsVDF(const std::wstring& mode, const std::wstring& name, const std::wstring& shortcutspath,
                                bool is_admin_try) const
{
#ifdef _WIN32
    const std::filesystem::path config_path = is_admin_try
                                                  ? shortcutspath
                                                  : std::wstring(getSteamPath()) + user_data_path_.toStdWString() +
                                                        getSteamUserId() + shortcutsfile_.toStdWString();

    qDebug() << "Steam config Path: " << config_path;
    qDebug() << "Trying to write config as admin: " << is_admin_try;

    bool write_res;
    try {
        write_res = VDFParser::Parser::writeShortcuts(config_path, shortcuts_vdf_, qDebug());
    }
    catch (const std::exception& e) {
        qDebug() << "Couldn't backup shortcuts file: " << e.what();
    }

    if (!write_res && !is_admin_try) {
        wchar_t szPath[MAX_PATH];
        if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
            // Launch itself as admin
            SHELLEXECUTEINFO sei = {sizeof(sei)};
            sei.lpVerb = L"runas";
            qDebug() << QString("exepath: %1").arg(szPath);
            sei.lpFile = szPath;
            const std::wstring paramstr = mode + L" " + name + L" \"" + config_path.wstring() + L"\"";
            sei.lpParameters = paramstr.c_str();
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            if (!ShellExecuteEx(&sei)) {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_CANCELLED) {
                    qDebug() << "User cancelled UAC Prompt";
                    return false;
                }
            }
            else {
                qDebug() << QString("HProc: %1").arg((int)sei.hProcess);

                if (sei.hProcess && WAIT_OBJECT_0 == WaitForSingleObject(sei.hProcess, INFINITE)) {
                    DWORD exitcode = 1;
                    GetExitCodeProcess(sei.hProcess, &exitcode);
                    qDebug() << QString("Exitcode: %1").arg((int)exitcode);
                    if (exitcode == 0) {
                        return true;
                    }
                }
                return false;
            }
        }
    }
    return write_res;
#else
    return VDFParser::Parser::writeShortcuts(config_path, shortcuts_vdf_);
#endif
}

bool UIModel::getIsDebug() const
{
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}

bool UIModel::getIsWindows() const { return is_windows_; }

bool UIModel::hasAcrylicEffect() const { return has_acrylic_affect_; }

void UIModel::setAcrylicEffect(bool has_acrylic_affect)
{
    has_acrylic_affect_ = has_acrylic_affect;
    emit acrylicChanged();
}

QStringList UIModel::getSteamgridOutput() const { return steamgrid_output_; }

void UIModel::onAvailFilesResponse(QNetworkReply* reply)
{

    const QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    qDebug() << "http status: " << status_code;
    if (status_code.isValid() && status_code.toInt() == 200) {
        const auto respStr = reply->readAll();
        qDebug() << "AvailFiles response: " << respStr;

        QJsonObject json = QJsonDocument::fromJson(respStr).object();

        const auto defaultConf = getDefaultConf();
        bool snapshotNotify =
            defaultConf.contains("snapshotNotify") ? defaultConf["snapshotNotify"].toJsonValue().toBool() : false;

        struct VersionInfo {
            int major;
            int minor;
            int patch;
            int revision;
            int commits_since_last;
        };

        std::vector<std::pair<QString, VersionInfo>> new_versions;
        for (const auto& info :
             json.keys() | std::ranges::views::filter([this, &json, snapshotNotify](const auto& key) {
                 return notify_on_snapshots_
                            ? true
                            : json[key].toObject().value("type") == (snapshotNotify ? "snapshot" : "release");
             }) | std::ranges::views::transform([&json](const auto& key) -> std::pair<QString, VersionInfo> {
                 const auto versionString = json[key].toObject().value("version").toString();
                 const auto cleanVersion = versionString.split("-")[0];
                 const auto versionSplits = cleanVersion.split(".");
                 return {key,
                         {versionSplits[0].toInt(), versionSplits[1].toInt(), versionSplits[2].toInt(),
                          versionSplits[3].toInt(),
                          versionString.count('-') == 2 ? versionString.split("-")[1].toInt() : 0}};
             }) | std::views::filter([](const auto& info) {
                 if (info.second.major > version::VERSION_MAJOR) {
                     return true;
                 }
                 if (info.second.major == version::VERSION_MAJOR && info.second.minor > version::VERSION_MINOR) {
                     return true;
                 }
                 if (info.second.major == version::VERSION_MAJOR && info.second.minor == version::VERSION_MINOR &&
                     info.second.patch > version::VERSION_PATCH) {
                     return true;
                 }
                 if (info.second.major == version::VERSION_MAJOR && info.second.minor == version::VERSION_MINOR &&
                     info.second.patch == version::VERSION_PATCH && info.second.revision > version::VERSION_REVISION) {
                     return true;
                 }
                 if (info.second.major == version::VERSION_MAJOR && info.second.minor == version::VERSION_MINOR &&
                     info.second.patch == version::VERSION_PATCH && info.second.revision == version::VERSION_REVISION &&
                     info.second.commits_since_last > (QString(version::VERSION_STR).count('-') == 2
                                                           ? QString(version::VERSION_STR).split("-")[1].toInt()
                                                           : 0)) {
                     return true;
                 }
                 return false;
             }) | std::ranges::views::all) {
            new_versions.push_back(info);
        }
        std::ranges::sort(new_versions, [](const auto& a, const auto& b) { return a.first > b.first; });
        if (!new_versions.empty()) {
            qDebug() << "New version available: " << new_versions[0].first;
            new_version_name_ = new_versions[0].first;
            emit newVersionAvailable();
        }
    }
}

void UIModel::onSteamGridReadReady()
{
    const auto output = QString::fromLocal8Bit(steamgrid_proc_.readAllStandardOutput());
    steamgrid_output_.push_back(output);
    emit steamgridOutputChanged();
    if (output.toLower().contains("token is missing or invalid")) {
        steamgrid_proc_.kill();
    }
}

void UIModel::writeTarget(const QJsonObject& json, const QString& name) const
{
    auto path = config_path_;
    path /= config_dir_name_.toStdWString();
    path /= (QString(name).replace(QRegularExpression("[\\\\/:*?\"<>|]"), "") + ".json").toStdWString();
    QFile file(path);
    if (!file.open(QIODevice::Text | QIODevice::ReadWrite)) {
        qDebug() << "Couldn't open file for writing: " << path;
        return;
    }

    auto json_ob = QJsonDocument(json).object();
    json_ob.remove("steamgridApiKey");

    file.write(QString(QJsonDocument(json_ob).toJson(QJsonDocument::Indented)).toStdString().data());
    file.close();
}

QString UIModel::getVersionString() const { return QString(version::VERSION_STR); }

QString UIModel::getNewVersionName() const { return new_version_name_; }

std::filesystem::path UIModel::getSteamPath(bool tryConfig) const
{
    QVariantMap defaultConf;
    if (tryConfig) {
        defaultConf = getDefaultConf();
    }   

    try {
#ifdef _WIN32
        // TODO: check if keys/value exist
        // steam should always be open and have written reg values...
        winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam"};
        if (!key.IsValid()) {
            if (defaultConf.contains("steamPath") &&
                QMetaType::canConvert(defaultConf["steamPath"].metaType(), QMetaType(QMetaType::QString))) {
                return defaultConf["steamPath"].toString().toStdWString();
            }
            return "";
        }
        const auto res = key.GetStringValue(L"SteamPath");
        return res;
    }
    catch (...) {
        if (defaultConf.contains("steamPath") &&
            QMetaType::canConvert(defaultConf["steamPath"].metaType(), QMetaType(QMetaType::QString))) {
            return defaultConf["steamPath"].toString().toStdWString();
        }
        return "";
    }
#else
        return L""; // TODO LINUX
#endif
}

std::wstring UIModel::getSteamUserId(bool tryConfig) const
{
    QVariantMap defaultConf;
    if (tryConfig) {
        defaultConf = getDefaultConf();
    }
#ifdef _WIN32
    try {
        // TODO: check if keys/value exist
        // steam should always be open and have written reg values...
        winreg::RegKey key{HKEY_CURRENT_USER, L"SOFTWARE\\Valve\\Steam\\ActiveProcess"};
        if (!key.IsValid()) {
            if (defaultConf.contains("steamUserId") &&
                QMetaType::canConvert(defaultConf["steamUserId"].metaType(), QMetaType(QMetaType::QString))) {
                return defaultConf["steamUserId"].toString().toStdWString();
            }
            return L"0";
        }
        const auto res = std::to_wstring(key.GetDwordValue(L"ActiveUser"));
        if (res == L"0") {
            qDebug() << "Steam not open?";
            if (defaultConf.contains("steamUserId") &&
                QMetaType::canConvert(defaultConf["steamUserId"].metaType(), QMetaType(QMetaType::QString))) {
                return defaultConf["steamUserId"].toString().toStdWString();
            }
        }
        return res;
    }
    catch (...) {
        if (defaultConf.contains("steamUserId") &&
            QMetaType::canConvert(defaultConf["steamUserId"].metaType(), QMetaType(QMetaType::QString))) {
            return defaultConf["steamUserId"].toString().toStdWString();
        }
        return L"0";
    }
#else
        return L""; // TODO LINUX
#endif
}

bool UIModel::foundSteam() const
{
    if (getSteamPath() == "" || getSteamUserId() == L"0") {
        return false;
    }
    const std::filesystem::path user_config_dir =
        std::wstring(getSteamPath()) + user_data_path_.toStdWString() + getSteamUserId();
    if (!std::filesystem::exists(user_config_dir)) {
        return false;
    }
    return true;
}

void UIModel::parseShortcutVDF()
{
    const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() +
                                              getSteamUserId() + shortcutsfile_.toStdWString();
    if (!std::filesystem::exists(config_path)) {
        qDebug() << "Shortcuts file does not exist.";
        return;
    }

    try {
        shortcuts_vdf_ = VDFParser::Parser::parseShortcuts(config_path, qDebug());
    }
    catch (const std::exception& e) {
        qDebug() << "Error parsing VDF: " << e.what();
    }
}

bool UIModel::isSteamInputXboxSupportEnabled() const
{
    // return true as default to not bug the user in error cases.
    if (foundSteam()) {
        const std::filesystem::path config_path = std::wstring(getSteamPath()) + user_data_path_.toStdWString() +
                                                  getSteamUserId() + user_config_file_.toStdWString();
        if (!std::filesystem::exists(config_path)) {
            qDebug() << "localconfig.vdf does not exist.";
            return true;
        }
        QFile file(config_path);
        if (file.open(QIODevice::Text | QIODevice::ReadOnly)) {
            QTextStream in(&file);
            QString line = in.readLine();
            // simple, regex approach should be enough...
            while (!in.atEnd()) {
                if (line.contains("SteamController_XBoxSupport")) {
                    file.close();
                    if (line.contains("1")) {
                        qDebug() << "\"SteamController_XBoxSupport\" is enabled!";
                        return true;
                    }
                    qDebug() << "\"SteamController_XBoxSupport\" is disabled!";
                    return false;
                }
                line = in.readLine();
            }
            qDebug() << "couldn't find \"SteamController_XBoxSupport\" in localconfig.vdf";
            file.close();
        }
        else {
            qDebug() << "could not open localconfig.vdf";
        }
    }
    return true;
}

void UIModel::readUnhookBytes() const
{
    std::map<std::string, std::string> unhook_bytes;
    for (const auto& name : UnhookUtil::UNHOOK_BYTES_ORIGINAL_22000 | std::views::keys) {
        auto bytes = UnhookUtil::ReadOriginalBytes(
            name,
            name.starts_with("Hid")
                ? L"hid.dll"
                   : name.starts_with("Setup") 
                        ? L"setupapi.dll"
                        : L"Kernel32.dll"
        );
        unhook_bytes[name] = bytes;
    }
    auto path = config_path_;
    path /= "unhook_bytes";
    QFile file(path);
    if (!file.open(QIODevice::Truncate | QIODevice::ReadWrite)) {
        qDebug() << "Couldn't open file for writing: " << path;
        return;
    }

    for (const auto& [name, bytes] : unhook_bytes) {
        file.write(
            QString::fromStdString(name + ":").toStdString().data()
        );
        file.write(bytes.data(), bytes.size());
        file.write("\n");
    }
    file.close();
}
