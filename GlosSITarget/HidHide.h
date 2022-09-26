/*
Copyright 2021-2022 Peter Repukat - FlatspotSoftware

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
#define NOMINMAX
#include <Windows.h>

#include <hidsdi.h>

#include <SetupAPI.h>
#include <array>
#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <SFML/System/Clock.hpp>

class HidHide {
  private:
    using DeviceInstancePath = std::wstring;
    using SetupDiDestroyDeviceInfoListPtr = std::unique_ptr<std::remove_pointer_t<HDEVINFO>, decltype(&SetupDiDestroyDeviceInfoList)>;
    using CloseHandlePtr = std::unique_ptr<std::remove_pointer_t<HANDLE>, decltype(&CloseHandle)>;
    using HidD_FreePreparsedDataPtr = std::unique_ptr<std::remove_pointer_t<PHIDP_PREPARSED_DATA>, decltype(&HidD_FreePreparsedData)>;

    // The Hid Hide I/O control custom device type (range 32768 .. 65535)
    static constexpr DWORD IoControlDeviceType = 32769;
    // The Hid Hide I/O control codes
    enum class IOCTL_TYPE : DWORD {
        GET_WHITELIST = CTL_CODE(IoControlDeviceType, 2048, METHOD_BUFFERED, FILE_READ_DATA),
        SET_WHITELIST = CTL_CODE(IoControlDeviceType, 2049, METHOD_BUFFERED, FILE_READ_DATA),
        GET_BLACKLIST = CTL_CODE(IoControlDeviceType, 2050, METHOD_BUFFERED, FILE_READ_DATA),
        SET_BLACKLIST = CTL_CODE(IoControlDeviceType, 2051, METHOD_BUFFERED, FILE_READ_DATA),
        GET_ACTIVE = CTL_CODE(IoControlDeviceType, 2052, METHOD_BUFFERED, FILE_READ_DATA),
        SET_ACTIVE = CTL_CODE(IoControlDeviceType, 2053, METHOD_BUFFERED, FILE_READ_DATA)
    };

    struct SmallHidInfo {
        std::wstring name;
        DeviceInstancePath device_instance_path;
        DeviceInstancePath base_container_device_instance_path;
        std::filesystem::path symlink;
        USHORT vendor_id;
        USHORT product_id;
        bool gaming_device = false;
    };

  public:
    HidHide();

    void openCtrlDevice();
    void closeCtrlDevice();

    void hideDevices(const std::filesystem::path& steam_path);
    void disableHidHide();
    // TODO: MAYBE: restore hidhide state/lists when app closes. not only disable device_hiding

  private:
    HANDLE hidhide_handle = nullptr;

    std::filesystem::path steam_path_;
    bool device_hiding_setup_ = false;

    static void UnPatchValveHooks();

    void enableOverlayElement();
    sf::Clock overlay_elem_clock_;

    std::vector<std::wstring> blacklisted_devices_;
    std::vector<SmallHidInfo> avail_devices_;
    bool hidhide_active_ = false;
    static constexpr int OVERLAY_ELEM_REFRESH_INTERVAL_S_ = 5; 

    static inline constexpr std::array<std::wstring_view, 3> whitelist_executeables_{
        L"GameOverlayUI.exe",
        L"steam.exe",
        L"streaming_client.exe"};

    static [[nodiscard]] std::wstring DosDeviceForVolume(const std::wstring& volume);

    [[nodiscard]] std::vector<std::wstring> getAppWhiteList() const;
    [[nodiscard]] std::vector<std::wstring> getBlackListDevices() const;
    [[nodiscard]] bool getActive();
    void setAppWhiteList(const std::vector<std::wstring>& whitelist) const;
    void setBlacklistDevices(const std::vector<std::wstring>& blacklist) const;
    void setActive(bool active);

    [[nodiscard]] DWORD getRequiredOutputBufferSize(IOCTL_TYPE type) const;

    static [[nodiscard]] std::vector<std::wstring> BufferToStringVec(const auto& buffer);
    static [[nodiscard]] std::vector<WCHAR> StringListToMultiString(const std::vector<std::wstring>& stringlist);

    static [[nodiscard]] std::vector<SmallHidInfo> GetHidDeviceList();
    static [[nodiscard]] SmallHidInfo GetDeviceInfo(const DeviceInstancePath& instance_path, const std::filesystem::path& symlink);
    static [[nodiscard]] bool DevicePresent(const DeviceInstancePath& dev);
    static [[nodiscard]] std::filesystem::path SymbolicLink(GUID const& interface_guid, DeviceInstancePath const& instance_path);
    static [[nodiscard]] DeviceInstancePath BaseContainerDeviceInstancePath(DeviceInstancePath const& device_instance_path);
    static [[nodiscard]] GUID BaseContainerId(DeviceInstancePath const& device_instance_path);
    static [[nodiscard]] DeviceInstancePath DeviceInstancePathParent(DeviceInstancePath const& device_instance_path);

    static [[nodiscard]] bool IsGamingDevice(const HIDD_ATTRIBUTES& attributes, const HIDP_CAPS& capabilities);
};
