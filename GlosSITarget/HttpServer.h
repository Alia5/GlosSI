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
#include <thread>

#include <httplib.h>

class AppLauncher;

class HttpServer {

  public:
    explicit HttpServer(AppLauncher& app_launcher, std::function<void()> close);

    void run();
    void stop();

  private:
    httplib::Server server_;
    std::thread server_thread_;
    uint16_t port_ = 8756;

    AppLauncher& app_launcher_;
    std::function<void()> close_;
};