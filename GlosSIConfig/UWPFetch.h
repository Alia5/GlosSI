#pragma once

#include <QVariant>
#include <VersionHelpers.h>
#include <Windows.h>
#include <appmodel.h>
#include <appxpackaging.h>
#include <collection.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <windows.h>
#pragma comment(lib, "Shlwapi.lib")
using namespace Windows::Management::Deployment;
using namespace Windows::Foundation::Collections;

namespace UWPFetch {

QVariantList UWPAppList()
{
    // TODO really should do this async, and notify gui when complete...
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
    // is it considered stealing when you take code that was pull-requested by someone else into your own repo?
    // Anyway... Stolen from: https://github.com/Thracky/GloSC/commit/3cd92e058498e3ab9d73ced140bbd7e490f639a7
    // https://github.com/Alia5/GloSC/commit/3cd92e058498e3ab9d73ced140bbd7e490f639a7

    // TODO: only return apps for current user.
    // TODO: I have no clue how this WinRT shit works; HELP MEH!

    // BTW: This config app needs to run as admin, in order to modify steams shortcuts.vdf
    // This won't hopefully be a problem fetching UWP apps for a specific user?

    PackageManager ^ packageManager = ref new PackageManager();
    IIterable<Windows::ApplicationModel::Package ^> ^ packages = packageManager->FindPackages();

    int packageCount = 0;
    // Only way to get the count of packages is to iterate through the whole collection first
    std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
                  [&](Windows::ApplicationModel::Package ^ package) {
                      packageCount += 1;
                  });

    int currPackage = 0;
    // Iterate through all the packages
    std::for_each(Windows::Foundation::Collections::begin(packages), Windows::Foundation::Collections::end(packages),
                  [&](Windows::ApplicationModel::Package ^ package) {
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
                                  0,     // default file attributes
                                  FALSE, // do not create new file
                                  NULL,  // no template
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
                                          for (auto& logoNameStr : logoNames) {
                                              application->GetStringValue(logoNameStr.c_str(), &iconName);
                                              if (!std::wstring(iconName).empty()) {
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
                              if (manifestAppName != NULL) {
                                  // If the display name is an indirect string, we'll try and load it using SHLoadIndirectString
                                  if (wcsstr(manifestAppName, L"ms-resource:")) {
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
                                  else {
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

                              free(pathBuf);

                              pairs.push_back(uwpPair);
                          }
                      }
                      currPackage += 1;
                  });
    return pairs;
}

} // namespace UWPFetch