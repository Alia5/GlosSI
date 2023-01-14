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
#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <QProcess>
#include <filesystem>
#include <shortcuts_vdf.hpp>

class QNetworkReply;

class UIModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isDebug READ getIsDebug CONSTANT)
    Q_PROPERTY(bool isWindows READ getIsWindows CONSTANT)
    Q_PROPERTY(bool hasAcrlyicEffect READ hasAcrylicEffect NOTIFY acrylicChanged)
    Q_PROPERTY(QVariantList targetList READ getTargetList NOTIFY targetListChanged)
    Q_PROPERTY(QVariantList uwpList READ uwpApps CONSTANT)
    Q_PROPERTY(QVariantList egsList READ egsGamesList CONSTANT)
    Q_PROPERTY(bool foundSteam READ foundSteam CONSTANT)
    Q_PROPERTY(bool steamInputXboxSupportEnabled READ isSteamInputXboxSupportEnabled CONSTANT)

    Q_PROPERTY(QString versionString READ getVersionString CONSTANT)
    Q_PROPERTY(QString newVersionName READ getNewVersionName NOTIFY newVersionAvailable)

    Q_PROPERTY(QStringList steamgridOutput READ getSteamgridOutput NOTIFY steamgridOutputChanged)

  public:
    UIModel();

    Q_INVOKABLE void readTargetConfigs();
    Q_INVOKABLE QVariantList getTargetList() const;
    Q_INVOKABLE void addTarget(QVariant shortcut);
    Q_INVOKABLE bool updateTarget(int index, QVariant shortcut);
    Q_INVOKABLE void deleteTarget(int index);
    Q_INVOKABLE bool isInSteam(QVariant shortcut) const;
    Q_INVOKABLE uint32_t getAppId(QVariant shortcut);
    Q_INVOKABLE bool addToSteam(QVariant shortcut, const QString& shortcutspath, bool from_cmd = false);
    bool addToSteam(const QString& name, const QString& shortcutspath, bool from_cmd = false);
    Q_INVOKABLE bool removeFromSteam(const QString& name, const QString& shortcutspath, bool from_cmd = false);
    Q_INVOKABLE QVariantMap manualProps(QVariant shortcut);
    Q_INVOKABLE void enableSteamInputXboxSupport();

    Q_INVOKABLE bool restartSteam(const QString& steamURL = "");

    Q_INVOKABLE void updateCheck();

    Q_INVOKABLE QVariantMap getDefaultConf() const;
    Q_INVOKABLE void saveDefaultConf(QVariantMap conf) const;
    
#ifdef _WIN32
    Q_INVOKABLE QVariantList uwpApps();
#endif
    Q_INVOKABLE QVariantList egsGamesList() const;

    Q_INVOKABLE void loadSteamGridImages();
    Q_INVOKABLE QString getGridImagePath(QVariant shortcut);

    [[nodiscard]] bool writeShortcutsVDF(const std::wstring& mode, const std::wstring& name,
                                         const std::wstring& shortcutspath, bool is_admin_try = false) const;

    bool getIsDebug() const;
    bool getIsWindows() const;
    [[nodiscard]] bool hasAcrylicEffect() const;
    void setAcrylicEffect(bool has_acrylic_affect);

    QStringList getSteamgridOutput() const;

  signals:
    void acrylicChanged();
    void targetListChanged();
    void newVersionAvailable();
    void steamgridOutputChanged();

  public slots:
    void onAvailFilesResponse(QNetworkReply* reply);
    void onSteamGridReadReady();

  private:
#ifdef _WIN32
    bool is_windows_ = true;
#else
    bool is_windows_ = false;
#endif
    bool has_acrylic_affect_ = false;

    std::filesystem::path config_path_;
    QString config_dir_name_;

    QString shortcutsfile_ = "/config/shortcuts.vdf";
    QString user_config_file_ = "/config/localconfig.vdf";
    QString user_data_path_ = "/userdata/";
    QString steam_executable_name_ = "steam.exe";

    const std::wstring_view egs_games_json_path_ =
        L"Epic/UnrealEngineLauncher/LauncherInstalled.dat";

    QVariantList targets_;

    QString new_version_name_;
    bool notify_on_snapshots_ = false;

    QProcess steamgrid_proc_;
    QStringList steamgrid_output_;

    std::vector<VDFParser::Shortcut> shortcuts_vdf_;

    void writeTarget(const QJsonObject& json, const QString& name) const;

    QString getVersionString() const;
    QString getNewVersionName() const;

    std::filesystem::path getSteamPath(bool tryConfig = true) const;
    std::wstring getSteamUserId(bool tryConfig = true) const;
    bool foundSteam() const;
    void parseShortcutVDF();

    bool isSteamInputXboxSupportEnabled() const;

    void readUnhookBytes() const;
};
