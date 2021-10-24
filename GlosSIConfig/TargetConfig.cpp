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
#include "TargetConfig.h"

TargetConfig::TargetConfig() : QObject(nullptr)
{
    
}

QString TargetConfig::getName() const
{
    return name_;
}

void TargetConfig::setName(const QString& name)
{
    name_ = name;
    emit nameChanged();
}

bool TargetConfig::getLaunch() const
{
    return launch_;
}

void TargetConfig::setLaunch(const bool launch)
{
    launch_ = launch;
}

QString TargetConfig::getLaunchPath() const
{
    return launch_path_;
}

void TargetConfig::setLaunchPath(const QString& launch_path)
{
    launch_path_ = launch_path;
}

QString TargetConfig::getLaunchAppArgs() const
{
    return launch_app_args_;
}

void TargetConfig::setLaunchAppArgs(const QString& launch_app_args)
{
    launch_app_args_ = launch_app_args;
}

bool TargetConfig::getCloseOnExit() const
{
    return close_on_exit_;
}

void TargetConfig::setCloseOnExit(const bool close_on_exit)
{
    close_on_exit_ = close_on_exit;
}

bool TargetConfig::getHideDevices() const
{
    return hide_devices_;
}

void TargetConfig::setHideDevices(const bool hide_devices)
{
    hide_devices_ = hide_devices;
}

bool TargetConfig::getWindowMode() const
{
    return window_mode_;
}

void TargetConfig::setWindowMode(const bool window_mode)
{
    window_mode_ = window_mode;
}

int TargetConfig::getMaxFps() const
{
    return max_fps_;
}

void TargetConfig::setMaxFps(const int max_fps)
{
    max_fps_ = max_fps;
}

float TargetConfig::getScale() const
{
    return scale_;
}

void TargetConfig::setScale(const float scale)
{
    scale_ = scale;
}
