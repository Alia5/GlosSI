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
#pragma once
#include <thread>

#include <httplib.h>
#include <nlohmann/json.hpp>

class AppLauncher;

class HttpServer {

  public:
    explicit HttpServer(std::function<void()> close);

    // C++ enums suck.
    enum Method {
        GET,
        POST,
        PUT,
        PATCH,
    };
    // but im not in the mood of adding yet another dependency for just that shit here.
    static std::string ToString(Method m);

    struct Endpoint {
        std::string path;
        Method method;
        std::function<void(const httplib::Request& req, httplib::Response& res)> handler;
        nlohmann::json response_hint = nullptr;
        nlohmann::json payload_hint = nullptr;
    };

    static void AddEndpoint(const Endpoint&& e);

    void run();
    void stop();


  private:
    httplib::Server server_;
    std::thread server_thread_;
    uint16_t port_ = 8756;

    std::function<void()> close_;

    static inline std::vector<Endpoint> endpoints_;

};