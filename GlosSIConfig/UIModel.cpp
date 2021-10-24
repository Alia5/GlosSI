#include "UIModel.h"

#include <QDir>

UIModel::UIModel()
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

QStringList UIModel::getTargetList() const
{
    QDir dir(config_dir_name_);
    auto entries = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    entries.removeIf([](const auto& entry) {
        return entry.endsWith(".json");
        });
    QStringList res;
    std::ranges::transform(entries, std::back_inserter(res), [](const auto& entry)
        {
            return entry.mid(0, entry.lastIndexOf(".json"));
        });
    res.push_back("Debug");
    return res;
}

bool UIModel::getIsWindows() const
{
    return is_windows_;
}
