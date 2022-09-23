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

// parts of code adapted from https://github.com/ViGEm/HidHide/blob/HEAD/HidHideCLI/src/HID.cpp
// // (c) Eric Korff de Gidts
// SPDX-License-Identifier: MIT

#include "HidHide.h"

#include <numeric>
#include <spdlog/spdlog.h>
#include <vector>

// Device configuration related
#include <cfgmgr32.h>

#include <initguid.h>
//
#include "Overlay.h"
#include "Settings.h"

#include <devguid.h>
#include <devpkey.h>
#include <regex>

// {D61CA365-5AF4-4486-998B-9DB4734C6CA3}add the XUSB class GUID as it is missing in the public interfaces
DEFINE_GUID(GUID_DEVCLASS_XUSBCLASS, 0xD61CA365, 0x5AF4, 0x4486, 0x99, 0x8B, 0x9D, 0xB4, 0x73, 0x4C, 0x6C, 0xA3);
// {EC87F1E3-C13B-4100-B5F7-8B84D54260CB} add the XUSB interface class GUID as it is missing in the public interfaces
DEFINE_GUID(GUID_DEVINTERFACE_XUSB, 0xEC87F1E3, 0xC13B, 0x4100, 0xB5, 0xF7, 0x8B, 0x84, 0xD5, 0x42, 0x60, 0xCB);
// {00000000-0000-0000-FFFF-FFFFFFFFFFFF} the system container id
DEFINE_GUID(GUID_CONTAINER_ID_SYSTEM, 0x00000000, 0x0000, 0x0000, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

HidHide::HidHide(){};

void HidHide::openCtrlDevice()
{
    hidhide_handle = CreateFile(
        L"\\\\.\\HidHide",
        GENERIC_READ,
        (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE),
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
}

void HidHide::closeCtrlDevice()
{
    if (hidhide_handle == nullptr) {
        return;
    }
    CloseHandle(hidhide_handle);
    hidhide_handle = nullptr;
}

void HidHide::hideDevices(const std::filesystem::path& steam_path)
{
    UnPatchValveHooks();

    openCtrlDevice();
    auto active = getActive();
    if (active) {
        // disable hidhide so we can see devices ourselves
        setActive(false);
    }
    auto whitelist = getAppWhiteList();
    // has anyone more than 4 keys to open overlay?!
    std::wsmatch m;
    const auto steam_path_string = steam_path.wstring();
    if (!std::regex_search(steam_path_string, m, std::wregex(L"(.:)(\\/|\\\\)")) || m.size() < 3) {
        spdlog::warn("Couldn't detect steam drive letter; Device hiding may not function");
        return;
    }
    const auto dos_device = DosDeviceForVolume(m[1]);
    if (dos_device.empty()) {
        spdlog::warn("Couldn't detect steam drive letter DOS Path; Device hiding may not function");
        return;
    }

    for (const auto& exe : whitelist_executeables_) {
        auto path = std::regex_replace(steam_path_string, std::wregex(L"(.:)(\\/|\\\\)"), dos_device + L"\\");
        path = std::regex_replace(path, std::wregex(L"\\/"), L"\\") + L"\\" + std::wstring{exe};
        if (std::ranges::none_of(whitelist, [&path](auto ep) { // make copy!
                auto p = path;                                 // non-const(!) copy of path
                std::ranges::transform(path, p.begin(), tolower);
                std::ranges::transform(ep, ep.begin(), tolower);
                return p == ep;
            })) {
            whitelist.push_back(path);
        }
    }
    if (Settings::extendedLogging) {
        std::ranges::for_each(whitelist, [](const auto& exe) {
            spdlog::trace(L"Whitelisted executable: {}", exe);
        });
    }
    setAppWhiteList(whitelist);

    avail_devices_ = GetHidDeviceList();
    blacklisted_devices_ = getBlackListDevices();

    for (const auto& dev : avail_devices_) {
        if (std::ranges::none_of(blacklisted_devices_, [&dev](const auto& blackdev) {
                return blackdev == dev.device_instance_path || blackdev == dev.base_container_device_instance_path;
            })) {
            // Valve emulated gamepad PID/VID; mirrord by ViGEm
            if (!(dev.vendor_id == 0x28de && dev.product_id == 0x11FF)) {
                if (!dev.device_instance_path.empty()) {
                    blacklisted_devices_.push_back(dev.device_instance_path);
                }
                if (!dev.device_instance_path.empty()) {
                    blacklisted_devices_.push_back(dev.base_container_device_instance_path);
                }
            }
        }
    }
    if (Settings::devices.hideDevices) {
        // TODO: MAXBE: remove all vigem controllers added by GlosSI
        setBlacklistDevices(blacklisted_devices_);
        setActive(true);
        spdlog::info("Hid Gaming Devices; Enabling Overlay element...");
        if (Settings::extendedLogging) {
            std::ranges::for_each(blacklisted_devices_, [](const auto& dev) {
                spdlog::trace(L"Blacklisted device: {}", dev);
            });
        }
        enableOverlayElement();
    }
    closeCtrlDevice();
}

void HidHide::disableHidHide()
{
    openCtrlDevice();
    if (getActive()) {
        setActive(false);
        spdlog::info("Un-hid Gaming Devices");
    }
    closeCtrlDevice();
}

void HidHide::UnPatchValveHooks()
{
    spdlog::debug("Unpatching Valve HID hooks...");
    // need to load addresses that way.. Otherwise we land before some jumps...
    if (const auto setupapidll = GetModuleHandle(L"setupapi.dll")) {
        UnPatchHook("SetupDiEnumDeviceInfo", setupapidll);
        UnPatchHook("SetupDiGetClassDevsW", setupapidll);
    }
    if (const auto hiddll = GetModuleHandle(L"hid.dll")) {
        for (const auto& name : ORIGINAL_BYTES | std::views::keys) {
            if (name.starts_with("Hid")) {
                UnPatchHook(name, hiddll);
            }            
        }
    }
}

void HidHide::UnPatchHook(const std::string& name, HMODULE module)
{
    spdlog::trace("Patching \"{}\"...", name);

    BYTE* address = reinterpret_cast<BYTE*>(GetProcAddress(module, name.c_str()));
    if (!address) {
        spdlog::error("failed to unpatch \"{}\"", name);
    }

    auto bytes = ORIGINAL_BYTES.at(name);
    DWORD dw_old_protect, dw_bkup;
    const auto len = bytes.size();
    VirtualProtect(address, len, PAGE_EXECUTE_READWRITE, &dw_old_protect); // Change permissions of memory..
    for (DWORD i = 0; i < len; i++)                                        //unpatch Valve's hook
    {
        *(address + i) = bytes[i];
    }
    VirtualProtect(address, len, dw_old_protect, &dw_bkup); // Revert permission change...
    spdlog::trace("Unpatched \"{}\"", name);
}

void HidHide::enableOverlayElement()
{
    Overlay::AddOverlayElem([this](bool window_has_focus) {
        ImGui::SetNextWindowPos({650, 100}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints({400, 270}, {1000, 1000});
        if (ImGui::Begin("Hidden Devices")) {
            if (window_has_focus && (overlay_elem_clock_.getElapsedTime().asSeconds() > OVERLAY_ELEM_REFRESH_INTERVAL_S_)) {
                // UnPatchValveHooks();
                openCtrlDevice();
                bool hidehide_state_store = hidhide_active_;
                if (Settings::extendedLogging) {
                    spdlog::debug("Refreshing HID devices");
                }
                if (hidhide_active_) {
                    setActive(false);
                }
                avail_devices_ = GetHidDeviceList();
                blacklisted_devices_ = getBlackListDevices();
                if (hidehide_state_store) {
                    setActive(true);
                }
                closeCtrlDevice();
                overlay_elem_clock_.restart();
            }
            ImGui::BeginChild("Inner", {0.f, ImGui::GetItemRectSize().y - 64}, true);
            std::ranges::for_each(avail_devices_, [this](const auto& device) {
                std::string label = (std::string(device.name.begin(), std::ranges::find(device.name, L'\0')) + "##" + std::string(device.device_instance_path.begin(), device.device_instance_path.end()));
                const auto findDeviceFn = [&device](const auto& blackdev) {
                    return device.device_instance_path == blackdev || device.base_container_device_instance_path == blackdev;
                };
                bool hidden = std::ranges::find_if(blacklisted_devices_, findDeviceFn) != blacklisted_devices_.end();
                if (ImGui::Checkbox(label.data(), &hidden)) {
                    openCtrlDevice();
                    if (hidden) {
                        if (std::ranges::none_of(blacklisted_devices_, findDeviceFn)) {
                            if (!device.device_instance_path.empty()) {
                                blacklisted_devices_.push_back(device.device_instance_path);
                            }
                            if (!device.device_instance_path.empty()) {
                                blacklisted_devices_.push_back(device.base_container_device_instance_path);
                            }
                        }
                    }
                    else {
                        blacklisted_devices_.erase(std::ranges::remove_if(blacklisted_devices_, findDeviceFn).begin(),
                                                   blacklisted_devices_.end());
                    }
                    setBlacklistDevices(blacklisted_devices_);
                    if (Settings::extendedLogging) {
                        std::ranges::for_each(blacklisted_devices_, [](const auto& dev) {
                            spdlog::trace(L"Blacklisted device: {}", dev);
                        });
                    }
                    closeCtrlDevice();
                }
            });
            ImGui::EndChild();
            if (ImGui::Checkbox("Devices Hidden", &hidhide_active_)) {
                openCtrlDevice();
                setActive(hidhide_active_);
                closeCtrlDevice();
            }
        }
        ImGui::End();
    });
}

std::wstring HidHide::DosDeviceForVolume(const std::wstring& volume)
{
    std::vector<WCHAR> buffer(UNICODE_STRING_MAX_CHARS);
    QueryDosDeviceW(volume.c_str(), buffer.data(), static_cast<DWORD>(buffer.size()));
    return {buffer.data()};
}

std::vector<std::wstring> HidHide::getAppWhiteList() const
{
    DWORD bytes_needed = getRequiredOutputBufferSize(IOCTL_TYPE::GET_WHITELIST);
    if (bytes_needed == 0) {
        return std::vector<std::wstring>{};
    }
    std::vector<WCHAR> buffer(bytes_needed);
    if (!DeviceIoControl(
            hidhide_handle, static_cast<DWORD>(IOCTL_TYPE::GET_WHITELIST), nullptr, 0, buffer.data(), static_cast<DWORD>(buffer.size() * sizeof(WCHAR)), &bytes_needed, nullptr)) {
        spdlog::error("Couldn't retrieve HidHide Whitelist");
        return std::vector<std::wstring>{};
    }
    return BufferToStringVec(buffer);
}

std::vector<std::wstring> HidHide::getBlackListDevices() const
{
    DWORD bytes_needed = getRequiredOutputBufferSize(IOCTL_TYPE::GET_BLACKLIST);
    if (bytes_needed == 0) {
        return std::vector<std::wstring>{};
    }
    std::vector<WCHAR> buffer(bytes_needed);
    if (!DeviceIoControl(
            hidhide_handle, static_cast<DWORD>(IOCTL_TYPE::GET_BLACKLIST), nullptr, 0, buffer.data(), static_cast<DWORD>(buffer.size() * sizeof(WCHAR)), &bytes_needed, nullptr)) {
        spdlog::error("Couldn't retrieve HidHide Blacklist");
        return std::vector<std::wstring>{};
    }
    return BufferToStringVec(buffer);
}

bool HidHide::getActive()
{
    DWORD bytes_needed;
    BOOLEAN res;
    if (!DeviceIoControl(
            hidhide_handle, static_cast<DWORD>(IOCTL_TYPE::GET_ACTIVE), nullptr, 0, &res, sizeof(BOOLEAN), &bytes_needed, nullptr)) {
        spdlog::error("Couldn't retrieve HidHide State");
        return false;
    }
    hidhide_active_ = res;
    return res;
}

void HidHide::setAppWhiteList(const std::vector<std::wstring>& whitelist) const
{
    DWORD bytes_needed;
    auto buffer = StringListToMultiString(whitelist);
    if (!DeviceIoControl(
            hidhide_handle, static_cast<DWORD>(IOCTL_TYPE::SET_WHITELIST), buffer.data(), static_cast<DWORD>(buffer.size() * sizeof(WCHAR)), nullptr, 0, &bytes_needed, nullptr)) {
        spdlog::error("Couldn't set HidHide WhiteList");
    }
}

void HidHide::setBlacklistDevices(const std::vector<std::wstring>& blacklist) const
{
    DWORD bytes_needed;
    auto buffer = StringListToMultiString(blacklist);
    if (!DeviceIoControl(
            hidhide_handle, static_cast<DWORD>(IOCTL_TYPE::SET_BLACKLIST), buffer.data(), static_cast<DWORD>(buffer.size() * sizeof(WCHAR)), nullptr, 0, &bytes_needed, nullptr)) {
        spdlog::error("Couldn't set HidHide BlackList");
    }
}

void HidHide::setActive(bool active)
{
    DWORD bytes_needed;
    if (!DeviceIoControl(
            hidhide_handle, static_cast<DWORD>(IOCTL_TYPE::SET_ACTIVE), &active, sizeof(BOOLEAN), nullptr, 0, &bytes_needed, nullptr)) {
        spdlog::error("Couldn't set HidHide State");
        return;
    }
    hidhide_active_ = active;
    if (Settings::extendedLogging) {
        spdlog::debug("HidHide State set to {}", active);
    }
}

DWORD HidHide::getRequiredOutputBufferSize(IOCTL_TYPE type) const
{
    DWORD bytes_needed;
    if (!DeviceIoControl(hidhide_handle, static_cast<DWORD>(type), nullptr, 0, nullptr, 0, &bytes_needed, nullptr)) {
        spdlog::error("Couldn't determine required HidHide output buffer size; type: {}", static_cast<int>(type));
        return 0;
    }
    return bytes_needed;
}

std::vector<std::wstring> HidHide::BufferToStringVec(const auto& buffer)
{
    std::vector<std::wstring> res;
    if (buffer[0] != L'\0') {
        res.emplace_back();
        for (const auto& ch : buffer) {
            if (ch == L'\0') {
                if ((res.end() - 1)->length() == 0) {
                    res.erase(res.end() - 1);
                    break;
                }
                res.emplace_back();
                continue;
            }
            (res.end() - 1)->push_back(ch);
        }
    }
    return res;
}

std::vector<WCHAR> HidHide::StringListToMultiString(const std::vector<std::wstring>& stringlist)
{
    auto res = std::accumulate(stringlist.begin(), stringlist.end(), std::vector<WCHAR>{}, [](auto acc, const auto& curr) {
        acc.insert(acc.end(), curr.begin(), curr.end());
        acc.push_back(L'\0');
        return acc;
    });
    res.push_back(L'\0');
    return res;
}

std::vector<HidHide::SmallHidInfo> HidHide::GetHidDeviceList()
{
    std::wstring hid_class_guid_string;
    hid_class_guid_string.resize(39);
    if (!StringFromGUID2(GUID_DEVCLASS_HIDCLASS, hid_class_guid_string.data(), static_cast<int>(hid_class_guid_string.size()))) {
        spdlog::error("couldn't convert GUID to string");
    }

    ULONG needed{};
    if (const auto result = CM_Get_Device_ID_List_SizeW(&needed, hid_class_guid_string.c_str(), CM_GETIDLIST_FILTER_CLASS);
        (CR_SUCCESS != result)) {
        spdlog::error("Couldn't get device id list size; code: {}", result);
    }
    std::vector<WCHAR> buffer(needed);
    if (const auto result = CM_Get_Device_ID_ListW(hid_class_guid_string.c_str(), buffer.data(), needed, CM_GETIDLIST_FILTER_CLASS);
        (CR_SUCCESS != result)) {
        spdlog::error("Couldn't get device id list; code: {}", result);
    }

    auto device_instance_paths = BufferToStringVec(buffer);

    device_instance_paths.erase(
        std::ranges::remove_if(
            device_instance_paths,
            [](const auto& dev) { return !DevicePresent(dev); })
            .begin(),
        device_instance_paths.end());

    GUID hid_device_interface_guid{};
    HidD_GetHidGuid(&hid_device_interface_guid);
    std::vector<SmallHidInfo> res;
    for (auto& instance_path : device_instance_paths) {
        auto symlink = SymbolicLink(hid_device_interface_guid, instance_path);
        if (!symlink.empty()) {
            res.push_back(GetDeviceInfo(instance_path, symlink));
        }
    }

    res.erase(
        std::ranges::remove_if(
            res,
            [](const auto& dev) { return !dev.gaming_device; })
            .begin(),
        res.end());

    return res;
}

HidHide::SmallHidInfo HidHide::GetDeviceInfo(const DeviceInstancePath& instance_path, const std::filesystem::path& symlink)
{
    SmallHidInfo res;
    res.device_instance_path = instance_path;

    // Open a handle to communicate with the HID device
    const CloseHandlePtr device_object(
        CreateFileW(
            symlink.c_str(),
            GENERIC_READ,
            (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE),
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr),
        &CloseHandle);
    if (INVALID_HANDLE_VALUE == device_object.get()) {
        const auto err = GetLastError();
        switch (err) {
        case ERROR_ACCESS_DENIED:
            // The device is opened exclusively and in use hence we can't interact with it
            __fallthrough;
        case ERROR_SHARING_VIOLATION:
            // The device is (most-likely) cloaked by Hid Hide itself while its client application isn't on the white-list
            __fallthrough;
        case ERROR_FILE_NOT_FOUND:
            // The device is currently not present hence we can't query its details
            return res;
        default:
            spdlog::error(L"Couldn't open device {}; code: {}", instance_path, err);
            return res;
        }
    }

    PHIDP_PREPARSED_DATA pre_parsed_data;
    if (!HidD_GetPreparsedData(device_object.get(), &pre_parsed_data)) {
        spdlog::error(L"Couldn't get PreParsed data; Device: {}", instance_path);
        return {};
    }
    const HidD_FreePreparsedDataPtr free_preparsed_data_ptr(pre_parsed_data, &HidD_FreePreparsedData);

    HIDP_CAPS capabilities;
    if (HIDP_STATUS_SUCCESS != HidP_GetCaps(pre_parsed_data, &capabilities)) {
        spdlog::error(L"Could get Hid capabilities; Device: {}", instance_path);
        return {};
    }

    HIDD_ATTRIBUTES attributes;
    if (!HidD_GetAttributes(device_object.get(), &attributes)) {
        spdlog::error(L"Could get Hid attributes; Device: {}", instance_path);
        return {};
    }
    std::wstring buffer;
    buffer.resize(127 * sizeof WCHAR);
    res.name = (HidD_GetProductString(device_object.get(), buffer.data(), static_cast<ULONG>(sizeof(WCHAR) * buffer.size()))
                    ? buffer
                    : L"");
    // Valve emulated gamepad PID/VID; mirrord by ViGEm
    if (attributes.VendorID == 0x28de /* && attributes.ProductID == 0x11FF*/) {
        res.name = std::wstring(L"ViGEm Emulated: ") + res.name;
    }
    res.base_container_device_instance_path = BaseContainerDeviceInstancePath(instance_path);
    res.gaming_device = IsGamingDevice(attributes, capabilities);
    res.vendor_id = attributes.VendorID;
    res.product_id = attributes.ProductID;

    return res;
}

bool HidHide::DevicePresent(const DeviceInstancePath& dev)
{
    DEVINST dev_inst{};
    if (
        const auto result = CM_Locate_DevNodeW(&dev_inst, const_cast<DEVINSTID_W>(dev.c_str()), CM_LOCATE_DEVNODE_NORMAL);
        (CR_NO_SUCH_DEVNODE == result) || (CR_SUCCESS == result)) {
        return (CR_SUCCESS == result);
    }
    spdlog::error(L"Couldn't determine if device \"{}\" is present", dev);
    return false;
}

std::filesystem::path HidHide::SymbolicLink(GUID const& interface_guid, DeviceInstancePath const& instance_path)
{
    // Ask the device for the device interface
    // Note that this call will succeed, whether or not the interface is present, but the iterator will have no entries, when the device interface isn't supported
    const SetupDiDestroyDeviceInfoListPtr handle(SetupDiGetClassDevsW(&interface_guid, instance_path.c_str(), nullptr, DIGCF_DEVICEINTERFACE), &SetupDiDestroyDeviceInfoList);
    if (INVALID_HANDLE_VALUE == handle.get()) {
        spdlog::error(L"Device Handle invalid; device: {}", instance_path);
        return {};
    }

    // Is the interface supported ?
    SP_DEVICE_INTERFACE_DATA device_interface_data;
    device_interface_data.cbSize = sizeof(device_interface_data);
    if (!SetupDiEnumDeviceInterfaces(handle.get(), nullptr, &interface_guid, 0, &device_interface_data)) {
        if (ERROR_NO_MORE_ITEMS != GetLastError()) {
            spdlog::error(L"Couldn't get Device interfaces; device: {}", instance_path);
            return {};
        }
        return {};
    }

    // Determine the buffer length needed
    DWORD needed{};
    if (!SetupDiGetDeviceInterfaceDetailW(handle.get(), &device_interface_data, nullptr, 0, &needed, nullptr) && ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
        spdlog::error(L"Couldn't get Device interface details; device: {}", instance_path);
        return {};
    }
    std::vector<BYTE> buffer(needed);

    // Acquire the detailed data containing the symbolic link (aka. device path)
    auto& [cbSize, DevicePath]{*reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(buffer.data())};
    cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
    if (!SetupDiGetDeviceInterfaceDetailW(handle.get(), &device_interface_data, reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(buffer.data()), static_cast<DWORD>(buffer.size()), nullptr, nullptr)) {
        spdlog::error(L"Couldn't get Device interface details; device: {}", instance_path);
        return {};
    }
    return {std::wstring(DevicePath)};
}

HidHide::DeviceInstancePath HidHide::BaseContainerDeviceInstancePath(DeviceInstancePath const& device_instance_path)
{
    const GUID base_container_id(BaseContainerId(device_instance_path));
    if ((GUID_NULL == base_container_id) || (GUID_CONTAINER_ID_SYSTEM == base_container_id))
        return (std::wstring{});
    for (auto it{device_instance_path};;) {
        if (const auto device_instance_path_parent = DeviceInstancePathParent(it); (base_container_id == BaseContainerId(device_instance_path_parent)))
            it = device_instance_path_parent;
        else
            return (it);
    }
}

GUID HidHide::BaseContainerId(DeviceInstancePath const& device_instance_path)
{
    // Bail out when the device instance path is empty
    if (device_instance_path.empty())
        return (GUID_NULL);

    DEVINST devInst{};
    DEVPROPTYPE devPropType{};
    GUID buffer{};
    ULONG needed{sizeof(buffer)};
    if (const auto result = CM_Locate_DevNodeW(&devInst, const_cast<DEVINSTID_W>(device_instance_path.c_str()), CM_LOCATE_DEVNODE_PHANTOM); (CR_SUCCESS != result)) {
        spdlog::error(L"Couldn't locate device DevNode; Device {}; Code: {}", device_instance_path, result);
        return {};
    }
    if (const auto result = CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_ContainerId, &devPropType, reinterpret_cast<PBYTE>(&buffer), &needed, 0); (CR_SUCCESS != result)) {
        // Bail out when the container id property isn't present
        if (CR_NO_SUCH_VALUE == result) {
            return (GUID_NULL);
        }
        spdlog::error(L"Couldn't locate device DevNode Property; Device {}; Code: {}", device_instance_path, result);
        return {};
    }
    if (DEVPROP_TYPE_GUID != devPropType) {
        spdlog::error(L"Device Prop is not GUID; Device {}", device_instance_path);
        return {};
    }
    return (buffer);
}

HidHide::DeviceInstancePath HidHide::DeviceInstancePathParent(DeviceInstancePath const& device_instance_path)
{
    DEVINST dev_inst{};
    DEVPROPTYPE dev_prop_type{};
    DEVINST dev_inst_parent{};
    std::wstring res;
    res.resize(UNICODE_STRING_MAX_CHARS);
    ULONG needed{static_cast<ULONG>(res.size())};
    if (const auto result = CM_Locate_DevNodeW(&dev_inst, const_cast<DEVINSTID_W>(device_instance_path.c_str()), CM_LOCATE_DEVNODE_PHANTOM); (CR_SUCCESS != result)) {
        spdlog::error(L"Couldn't locate device DevNode; Device {}; Code: {}", device_instance_path, result);
        return {};
    }
    if (const auto result = CM_Get_Parent(&dev_inst_parent, dev_inst, 0); (CR_SUCCESS != result)) {
        spdlog::error(L"Couldn't get device Parent; Device {}; Code: {}", device_instance_path, result);
        return {};
    }
    if (const auto result = CM_Get_DevNode_PropertyW(dev_inst_parent, &DEVPKEY_Device_InstanceId, &dev_prop_type, reinterpret_cast<PBYTE>(res.data()), &needed, 0); (CR_SUCCESS != result)) {
        spdlog::error(L"Couldn't locate device DevNode Property; Device {}; Code: {}", device_instance_path, result);
        return {};
    }
    if (DEVPROP_TYPE_STRING != dev_prop_type) {
        spdlog::error(L"Device Prop is not STRING; Device {}", device_instance_path);
        return {};
    }
    return res;
}

bool HidHide::IsGamingDevice(const HIDD_ATTRIBUTES& attributes, const HIDP_CAPS& capabilities)
{
    return (
        // 0x28DE 0x1142 = Valve Corporation Steam Controller
        // keep them for now
        /* ((attributes.VendorID == 0x28DE) && (attributes.ProductID == 0x1142)) || */
        (0x05 == capabilities.UsagePage) || (0x01 == capabilities.UsagePage) && ((0x04 == capabilities.Usage) || (0x05 == capabilities.Usage)));
}
