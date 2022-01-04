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
#include "VDFParser.h"
#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include <filesystem>

class UIModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isWindows READ getIsWindows CONSTANT)
    Q_PROPERTY(bool hasAcrlyicEffect READ hasAcrylicEffect NOTIFY acrylicChanged)
    Q_PROPERTY(QVariantList targetList READ getTargetList NOTIFY targetListChanged)
    Q_PROPERTY(QVariantList uwpList READ uwpApps CONSTANT)

  public:
    UIModel();

    Q_INVOKABLE void readConfigs();
    Q_INVOKABLE QVariantList getTargetList() const;
    Q_INVOKABLE void addTarget(QVariant shortcut);
    Q_INVOKABLE void updateTarget(int index, QVariant shortcut);
    Q_INVOKABLE void deleteTarget(int index);
    Q_INVOKABLE bool isInSteam(QVariant shortcut);
    Q_INVOKABLE bool addToSteam(QVariant shortcut, const QString& shortcutspath, bool from_cmd = false);
    bool addToSteam(const QString& name, const QString& shortcutspath, bool from_cmd = false);
    Q_INVOKABLE bool removeFromSteam(const QString& name, const QString& shortcutspath, bool from_cmd = false);
#ifdef _WIN32
    Q_INVOKABLE QVariantList uwpApps();
#endif

    [[nodiscard]] bool writeShortcutsVDF(
        const std::wstring& mode,
        const std::wstring& name,
        const std::wstring& shortcutspath,
        bool is_admin_try = false
    ) const;

    bool getIsWindows() const;
    [[nodiscard]] bool hasAcrylicEffect() const;
    void setAcrylicEffect(bool has_acrylic_affect);

  signals:
    void acrylicChanged();
    void targetListChanged();

  private:
    std::filesystem::path config_path_;
    QString config_dir_name_;
    
    void writeTarget(const QJsonObject& json, const QString& name);

    std::filesystem::path getSteamPath() const;
    std::wstring getSteamUserId() const;
    void parseShortcutVDF();
    QString shortcutsfile_ = "/config/shortcuts.vdf";
    QString user_data_path_ = "/userdata/";

    QVariantList targets_;

    VDFParser::VDFFile shortcuts_vdf_;

#ifdef _WIN32
    bool is_windows_ = true;
#else
    bool is_windows_ = false;
#endif
    bool has_acrylic_affect_ = false;
};
