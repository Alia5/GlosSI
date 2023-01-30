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
#include "InputRedirector.h"

#include <SFML/System/Clock.hpp>
#include <SFML/System/Sleep.hpp>
#include <spdlog/spdlog.h>

#include "Overlay.h"
#include "..\common\Settings.h"

InputRedirector::InputRedirector()
{
#ifdef _WIN32
    driver_ = vigem_alloc();
    vigem_connected_ = VIGEM_SUCCESS(vigem_connect(driver_));
    if (vigem_connected_) {
        spdlog::debug("Connected to ViGEm");
    }
    else {
        spdlog::error("Error initializing ViGEm");
    }
#endif
}

InputRedirector::~InputRedirector()
{
#ifdef _WIN32
    if (controller_thread_.joinable())
        controller_thread_.join();
    vigem_disconnect(driver_);
    vigem_free(driver_);
#endif
}

void InputRedirector::run()
{
    run_ = vigem_connected_;
    controller_thread_ = std::thread(&InputRedirector::runLoop, this);
#ifdef _WIN32
    Overlay::AddOverlayElem([this](bool window_has_focus, ImGuiID dockspace_id) {
        ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);
        ImGui::Begin("Controller Emulation");
        int countcopy = Settings::controller.maxControllers;
        ImGui::Text("Max. controller count");
        ImGui::SameLine();
        ImGui::InputInt("##Max. controller count", &countcopy, 1, 1);
        if (countcopy > XUSER_MAX_COUNT) {
            countcopy = XUSER_MAX_COUNT;
        }
        if (countcopy < 0) {
            countcopy = 0;
        }
        Settings::controller.maxControllers = countcopy;

        if (ImGui::Checkbox("Emulate DS4 (instead of Xbox360 controller)", &Settings::controller.emulateDS4)) {
            controller_settings_changed_ = true;
        }

        ImGui::Spacing();

        if (Settings::launch.launch) {
            ImGui::Checkbox("Allow desktop config", &Settings::controller.allowDesktopConfig);
            ImGui::Text("Allows desktop config if the launched application is not focused");

            ImGui::Spacing();
        }

        bool enable_rumbe_copy = enable_rumble_;
        ImGui::Checkbox("Enable Rumble", &enable_rumbe_copy);
        enable_rumble_ = enable_rumbe_copy;

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Advanced", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("GlosSI uses different USB-IDs for it's emulated controllers");
            ImGui::Text("This can cause issues with some games");
            ImGui::Text("When changing this, it's advised to also set Max. controller count");
            ImGui::Text("to prevent having multiple mirrored controllers plugged in");
            ImGui::Spacing();
            ImGui::Text("If enabled, Device hiding won't work! Use \"Max. Controller count\"-setting.");
            bool use_real_copy = Settings::devices.realDeviceIds;
            ImGui::Checkbox("Use real USB-IDs", &use_real_copy);
            if (Settings::devices.realDeviceIds != use_real_copy) {
                controller_settings_changed_ = true;
            }
            Settings::devices.realDeviceIds = use_real_copy;
        }
        ImGui::End();
    });
#endif
}

void InputRedirector::stop()
{
    run_ = false;
    controller_thread_.join();
    if (vigem_connected_) {
        for (const auto& target : vt_pad_) {
            vigem_target_remove(driver_, target);
        }
    }
}

void InputRedirector::runLoop()
{
    // wait for steam to do all of it's hooking
    const sf::Clock clock;
    while (clock.getElapsedTime().asMilliseconds() < start_delay_ms_) {
        sf::sleep(sf::milliseconds(20));
    }
    while (run_) {
#ifdef _WIN32
        if (controller_settings_changed_) {
            // unplug all.
            controller_settings_changed_ = false;
            for (int i = 0; i < XUSER_MAX_COUNT; i++) {
                unplugVigemPad(i);
            }
        }
        if (Settings::controller.maxControllers < XUSER_MAX_COUNT) {
            for (int i = Settings::controller.maxControllers; i < XUSER_MAX_COUNT; i++) {
                unplugVigemPad(i);
            }
        }
        for (int i = 0; i < XUSER_MAX_COUNT && i < Settings::controller.maxControllers; i++) {
            XINPUT_STATE state{};
            if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                if (vt_pad_[i] != nullptr) {
                    if (Settings::controller.emulateDS4) {
                        DS4_REPORT rep;
                        DS4_REPORT_INIT(&rep);

                        // The DualShock 4 expects a different report format, so we call a helper
                        // function which will translate buttons and axes 1:1 from XUSB to DS4
                        // format and submit it to the update function afterwards.
                        XUSB_TO_DS4_REPORT(reinterpret_cast<PXUSB_REPORT>(&state.Gamepad), &rep);
                        vigem_target_ds4_update(driver_, vt_pad_[i], rep);
                    }
                    else {
                        vigem_target_x360_update(driver_, vt_pad_[i], *reinterpret_cast<XUSB_REPORT*>(&state.Gamepad));
                    }
                }
                else {
                    if (Settings::controller.emulateDS4) {
                        vt_pad_[i] = vigem_target_ds4_alloc();
                    }
                    else {
                        vt_pad_[i] = vigem_target_x360_alloc();
                    }

                    // By using VID and PID of Valve's Emulated Controller
                    // ( https://partner.steamgames.com/doc/features/steam_controller/steam_input_gamepad_emulation_bestpractices )
                    // Steam doesn't give us ANOTHER "fake" XInput device
                    // -> Leading to endless pain and suffering.
                    // Or really, leading to plugging in one virtual controller after another and mirroring inputs
                    // Also annoying the shit out of the user when they open the overlay as steam prompts to setup new XInput devices
                    // Also avoiding any fake inputs from Valve's default controller profile
                    // -> Leading to endless pain and suffering
                    //
                    // Additonaly, Steam does pick up the emulated controller this way, and Hides it from our application
                    // but Steam ONLY does this if it is configured to support X360 controller rebinding!!!
                    // Otherwise, this application (GloSC/GlosSI) will pickup the emulated controller as well!
                    // This however is configurable withon GlosSI overlay;
                    // Multiple controllers can be worked around with by setting max count.
                    if (!Settings::devices.realDeviceIds) {
                        vigem_target_set_vid(vt_pad_[i], 0x28de); // VALVE_DIRECTINPUT_GAMEPAD_VID
                        // vigem_target_set_pid(vt_pad_[i], 0x11FF); //VALVE_DIRECTINPUT_GAMEPAD_PID
                        if (Settings::controller.emulateDS4) {
                            vigem_target_set_pid(vt_pad_[i], 0x05C4); // DS4 Controller
                        }
                        else {
                            vigem_target_set_pid(vt_pad_[i], 0x028E); // XBOX 360 Controller
                        }
                    }
                    else {
                        if (Settings::controller.emulateDS4) {
                            vigem_target_set_vid(vt_pad_[i], 0x054C); // Sony Corp.
                            vigem_target_set_pid(vt_pad_[i], 0x05C4); // DS4 Controller
                        }
                        else {
                            vigem_target_set_vid(vt_pad_[i], 0x045E); // MICROSOFT
                            vigem_target_set_pid(vt_pad_[i], 0x028E); // XBOX 360 Controller
                        }
                    }
                    // TODO: MAYBE!: In a future version, use something like OpenXInput
                    // and filter out emulated controllers to support a greater amount of controllers simultaneously

                    const int target_add_res = vigem_target_add(driver_, vt_pad_[i]);
                    if (target_add_res == VIGEM_ERROR_TARGET_UNINITIALIZED) {
                        if (Settings::controller.emulateDS4) {
                            vt_pad_[i] = vigem_target_ds4_alloc();
                        }
                        else {
                            vt_pad_[i] = vigem_target_x360_alloc();
                        }
                    }
                    if (target_add_res == VIGEM_ERROR_NONE) {
                        spdlog::info("Plugged in controller {}, {}; VID: {:x}; PID: {:x}",
                                     i,
                                     vigem_target_get_index(vt_pad_[i]),
                                     vigem_target_get_vid(vt_pad_[i]),
                                     vigem_target_get_pid(vt_pad_[i]));

                        if (Settings::controller.emulateDS4) {
                            // TODO: make sense of DS4_OUTPUT_BUFFER
                            // there is no doc? Ask @Nef about this...
                            // ReSharper disable once CppDeprecatedEntity
#pragma warning(disable : 4996)
                            const auto callback_register_res = vigem_target_ds4_register_notification(
                                driver_,
                                vt_pad_[i],
                                &InputRedirector::ds4ControllerCallback,
                                reinterpret_cast<LPVOID>(i));
                            if (!VIGEM_SUCCESS(callback_register_res)) {
                                spdlog::error("Registering controller {}, {} for notification failed with error code: {:#x}", i, vigem_target_get_index(vt_pad_[i]), callback_register_res);
                            }
                        }
                        else {
                            const auto callback_register_res = vigem_target_x360_register_notification(
                                driver_,
                                vt_pad_[i],
                                &InputRedirector::x360ControllerCallback,
                                reinterpret_cast<LPVOID>(i));
                            if (!VIGEM_SUCCESS(callback_register_res)) {
                                spdlog::error("Registering controller {}, {} for notification failed with error code: {:#x}", i, vigem_target_get_index(vt_pad_[i]), callback_register_res);
                            }
                        }
                    }
                }
            }
            else {
                unplugVigemPad(i);
            }
        }
        sf::sleep(sf::milliseconds(4));

#endif
    }
}

#ifdef _WIN32
void InputRedirector::x360ControllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData)
{
    if (!enable_rumble_) {
        return;
    }
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    vibration.wLeftMotorSpeed = LargeMotor * 257;
    vibration.wRightMotorSpeed = SmallMotor * 257;

    XInputSetState(reinterpret_cast<int>(UserData), &vibration);
}

void InputRedirector::unplugVigemPad(int idx)
{
    if (vt_pad_[idx] != nullptr) {
        if (VIGEM_SUCCESS(vigem_target_remove(driver_, vt_pad_[idx]))) {
            spdlog::info("Unplugged controller {}, {}", idx, vigem_target_get_index(vt_pad_[idx]));
            vt_pad_[idx] = nullptr;
        }
    }
}

void InputRedirector::ds4ControllerCallback(PVIGEM_CLIENT client, PVIGEM_TARGET Target, UCHAR LargeMotor, UCHAR SmallMotor, DS4_LIGHTBAR_COLOR LightbarColor, LPVOID UserData)
{
    if (!enable_rumble_) {
        return;
    }
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    vibration.wLeftMotorSpeed = LargeMotor * 257;
    vibration.wRightMotorSpeed = SmallMotor * 257;

    XInputSetState(reinterpret_cast<int>(UserData), &vibration);
}
#endif
