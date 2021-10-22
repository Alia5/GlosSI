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
#define NOMINMAX
#include <Windows.h>

#include <hidsdi.h>

#include <SetupAPI.h>
#include <array>
#include <filesystem>
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

    // Valve Hooks `SetupDiEnumDeviceInfo` and hides Gaming devices like this.
    // To be able to query them, unpatch the hook with the original bytes...
    static inline const std::string SETUP_DI_ENUM_DEV_INFO_ORIG_BYTES = "\x48\x89\x5C\x24\x08";
    // Valve also Hooks `SetupDiGetClassDevsW` ..unhook that as well...
    static inline const std::string SETUP_DI_GETCLASSDEVSW_ORIG_BYTES = "\x48\x89\x5C\x24\x08";
    // Valve also Hooks `HidD_GetPreparsedData` ..unhook that as well...
    static inline const std::string HID_GETPREPARSED_ORIG_BYTES = "\x48\x89\x5C\x24\x18";
    // ..aand `HidP_GetCaps`
    static inline const std::string HID_GETCAPS_ORIG_BYTES = "\x4C\x8B\xD1\x48\x85\xC9";
    // ...aaand `HidD_GetAttributes`
    static inline const std::string HID_GETATTRS_ORIG_BYTES = "\x40\x53\x48\x83\xEC";
    // ...aaaaand `HidD_GetProductString`
    static inline const std::string HID_GETPRODSTR_ORIG_BYTES = "\x48\x83\xEC\x48\x48";
    // ...aaaaand `HidP_GetUsages`
    static inline const std::string HID_GETUSAGE_ORIG_BYTES = "\x4C\x89\x4C\x24\x20";
    // ...aaaaand `HidP_GetData`
    static inline const std::string HID_GETDATA_ORIG_BYTES = "\x4C\x89\x44\x24\x18";
    // ...aaaaand `HidP_GetValueCaps`
    static inline const std::string HID_GETVALUECAPS_ORIG_BYTES = "\x48\x83\xEC\x48\x49";
    // ...aaaaand `HidP_GetUsageValue`
    static inline const std::string HID_GETUSAGE_VAL_ORIG_BYTES = "\x40\x53\x55\x56\x48";
    // ...aaaaand `HidP_GetButtonCaps`
    static inline const std::string HID_GETBTNCAPS_VAL_ORIG_BYTES = "\x48\x83\xEC\x48\x49";

    static void UnPatchValveHooks();
    static void UnPatchHook(BYTE* address, const std::string& bytes);

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
