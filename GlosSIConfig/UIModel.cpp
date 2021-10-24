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
#include <QJsonObject>
#include <QJsonDocument>


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

    readConfigs();
}

void UIModel::readConfigs()
{
    QDir dir(config_dir_name_);
    auto entries = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    entries.removeIf([](const auto& entry) {
        return !entry.endsWith(".json");
        });

    std::ranges::for_each(entries, [this](const auto& name)
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
            const auto json = jsondoc.object();
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
    // TODO: write config
    const auto map = shortcut.toMap();
    const auto json = QJsonDocument(QJsonObject::fromVariantMap(map));
    auto wtf = json.toJson(QJsonDocument::Indented).toStdString();

    writeTarget(wtf, map["name"].toString());

    targets_.append(json.toVariant());
    emit targetListChanged();
}

void UIModel::updateTarget(int index, QVariant shortcut)
{
    const auto map = shortcut.toMap();
    const auto json = QJsonDocument(QJsonObject::fromVariantMap(map));
    auto wtf = json.toJson(QJsonDocument::Indented).toStdString();

    writeTarget(wtf, map["name"].toString());

    auto oldName = targets_[index].toMap()["name"].toString() + ".json";
    auto path = config_path_;
    path /= config_dir_name_.toStdString();
    path /= (oldName).toStdString();
    std::filesystem::remove(path);

    targets_.replace(index, json.toVariant());
    emit targetListChanged();
}

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

void UIModel::writeTarget(const std::string& json, const QString& name)
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
    file.write(json.data());
    file.close();
}

