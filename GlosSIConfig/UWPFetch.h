#pragma once

#include <QProcess>
#include <QVariant>
#include <QVariantMap>

#include <VersionHelpers.h>
#include <Windows.h>

namespace UWPFetch {

QVariantList UWPAppList()
{
    if (!IsWindows10OrGreater()) {
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

    QProcess proc;
    proc.setProgram("powershell.exe");
    QStringList args;
    args.push_back("-noprofile");
    args.push_back("-ExecutionPolicy");
    args.push_back("Bypass");
    args.push_back("-File");
    args.push_back(".\\GetAUMIDs.ps1");
    proc.setArguments(args);
    proc.start();
    proc.waitForFinished(60000);
    const auto baseList = QString(proc.readAllStandardOutput()).split(";");
    QVariantList list;
    for (const auto& entry : baseList) {
        auto subList = entry.split('|');
        if (subList.size() < 4)
            continue;
        auto name = QString(subList[0]).replace("\r\n", "");
        if (name == "-Error-")
            continue;
        QVariantMap uwpPair;
        uwpPair.insert("AppName", name);
        uwpPair.insert("Path", subList[1]);
        uwpPair.insert("AppUMId", subList[3]);

        QString icoFName = subList[2];
        std::filesystem::path icoPath(icoFName.toStdString());

        std::vector<QString> possibleextensions = {".scale-100", ".scale-125", ".scale-150", ".scale-200"};
        if (!std::filesystem::exists(icoPath)) {
            for (const auto& ext : possibleextensions) {
                QString maybeFname = QString(icoFName).replace(".png", ext + ".png");
                std::filesystem::path maybePath(maybeFname.toStdString());
                if (std::filesystem::exists(maybePath)) {
                    icoPath = maybePath;
                    break;
                }
            }
        }

        uwpPair.insert("IconPath", QString::fromStdString(icoPath.string()));

        list.push_back(uwpPair);
    }
    if (list.empty()) {
        auto stderrstr = proc.readAllStandardError();
        auto stdoutstr = proc.readAllStandardOutput();
        list.emplaceBack(QVariantMap{
            {"AppName", "Error executing \"GetAUMIDs.ps1\""},
            {"Path", ""},
            {"AppUMId", QString::number(proc.error()) + ":" + stderrstr},
        });
    }
    return list;
}

} // namespace UWPFetch