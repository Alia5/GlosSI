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
#include <QObject>
#include <QVariant>
class TargetConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool launch READ getLaunch WRITE setLaunch NOTIFY launchChanged)
    Q_PROPERTY(QString launchPath READ getLaunchPath WRITE setLaunchPath NOTIFY launchPathChanged)
    Q_PROPERTY(QString launchAppArgs READ getLaunchAppArgs WRITE setLaunchAppArgs NOTIFY launchAppArgsChanged)
    Q_PROPERTY(bool closeOnExit READ getCloseOnExit WRITE setCloseOnExit NOTIFY closeOnExitChanged)
    Q_PROPERTY(bool hideDevices READ getHideDevices WRITE setHideDevices NOTIFY hideDevicesChanged)
    Q_PROPERTY(bool windowMode READ getWindowMode WRITE setWindowMode NOTIFY windowModeChanged)
    Q_PROPERTY(int maxFps READ getMaxFps WRITE setMaxFps NOTIFY maxFpsChanged)
    Q_PROPERTY(float scale READ getScale WRITE setScale NOTIFY scaleChanged)
public:
    TargetConfig();

    [[nodiscard]] QString getName() const;
    void setName(const QString& name);
    [[nodiscard]] bool getLaunch() const;
    void setLaunch(const bool launch);
    [[nodiscard]] QString getLaunchPath() const;
    void setLaunchPath(const QString& launch_path);
    [[nodiscard]] QString getLaunchAppArgs() const;
    void setLaunchAppArgs(const QString& launch_app_args);
    [[nodiscard]] bool getCloseOnExit() const;
    void setCloseOnExit(const bool close_on_exit);
    [[nodiscard]] bool getHideDevices() const;
    void setHideDevices(const bool hide_devices);
    [[nodiscard]] bool getWindowMode() const;
    void setWindowMode(const bool window_mode);
    [[nodiscard]] int getMaxFps() const;
    void setMaxFps(const int max_fps);
    [[nodiscard]] float getScale() const;
    void setScale(const float scale);


signals:
    void nameChanged();
    void launchChanged();
    void launchPathChanged();
    void launchAppArgsChanged();
    void closeOnExitChanged();
    void hideDevicesChanged();
    void windowModeChanged();
    void maxFpsChanged();
    void scaleChanged();


private:
    static inline constexpr int version_ = 1;
    QString name_;
    bool launch_ = true;
    QString launch_path_;
    QString launch_app_args_;
    bool close_on_exit_ = true;
    bool hide_devices_ = true;
    bool window_mode_ = false;
    int max_fps_ = 0;
    float scale_ = 0;

};

