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

#include "TargetConfig.h"

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
}

void UIModel::readConfigs()
{
    QDir dir(config_dir_name_);
    auto entries = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    entries.removeIf([](const auto& entry) {
        return entry.endsWith(".json");
        });
    QStringList fileNames;
    std::ranges::transform(fileNames, std::back_inserter(fileNames), [](const auto& entry)
        {
            return entry.mid(0, entry.lastIndexOf(".json"));
        });

    std::ranges::for_each(fileNames, [this](const auto& name)
        {
            targets_.append(QMap<QString, QVariant>{ { "name", name }});
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
    auto map = shortcut.toMap();

    QVariantMap copy;
    copy.insert("name", map["name"].toString());
    copy.insert("launch", map["launch"].toBool());
    copy.insert("launchPath", map["launchPath"].toString());
    copy.insert("launchAppArgs", map["launchAppArgs"].toString());
    copy.insert("closeOnExit", map["closeOnExit"].toBool());
    copy.insert("hideDevices", map["hideDevices"].toBool());
    copy.insert("windowMode", map["windowMode"].toBool());
    copy.insert("maxFps", map["maxFps"].toInt());
    copy.insert("scale", map["scale"].toInt());

    targets_.append(copy);
    emit targetListChanged();
}

void UIModel::updateTarget(int index, QVariant shortcut)
{

    auto map = shortcut.toMap();

    QVariantMap copy;
    copy.insert("name", map["name"].toString());
    copy.insert("launch", map["launch"].toBool());
    copy.insert("launchPath", map["launchPath"].toString());
    copy.insert("launchAppArgs", map["launchAppArgs"].toString());
    copy.insert("closeOnExit", map["closeOnExit"].toBool());
    copy.insert("hideDevices", map["hideDevices"].toBool());
    copy.insert("windowMode", map["windowMode"].toBool());
    copy.insert("maxFps", map["maxFps"].toInt());
    copy.insert("scale", map["scale"].toInt());

    targets_.replace(index, copy);
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
