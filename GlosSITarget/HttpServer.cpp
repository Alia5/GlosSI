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
#include "HttpServer.h"

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "AppLauncher.h"
#include "Settings.h"

HttpServer::HttpServer(AppLauncher& app_launcher, std::function<void()> close) : app_launcher_(app_launcher), close_(close)
{
}

void HttpServer::run()
{
    server_.Get("/launched-pids", [this](const httplib::Request& req, httplib::Response& res) {
        const nlohmann::json j = app_launcher_.launchedPids();
        res.set_content(j.dump(), "text/json");
    });

    server_.Post("/launched-pids", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            const nlohmann::json postbody = nlohmann::json::parse(req.body);
            app_launcher_.addPids(postbody.get<std::vector<DWORD>>());   
        } catch (std::exception& e) {
            res.status = 401;
            res.set_content(nlohmann::json{
                                {"code", 401},
                                {"name", "Bad Request"},
                                {"message", e.what()},
                            }
                                .dump(),
                            "text/json");
            return;
        }
        catch (...) {
            res.status = 500;
            res.set_content(nlohmann::json{
                                {"code", 500},
                                {"name", "Internal Server Error"},
                                {"message", "Unknown Error"},
                            }
                                .dump(),
                            "text/json");
            return;
        }
        const nlohmann::json j = app_launcher_.launchedPids();
        res.set_content(j.dump(), "text/json");
    });

    server_.Post("/quit", [this](const httplib::Request& req, httplib::Response& res) {
        close_();
    });

    server_.Get("/settings", [this](const httplib::Request& req, httplib::Response& res) {
        res.set_content(Settings::toJson().dump(), "text/json");
    });

    server_thread_ = std::thread([this]() {
        if (!server_.listen("0.0.0.0", port_)) {
            spdlog::error("Couldn't start http-server");
            return;
        }
        spdlog::debug("Started http-server on port {}", static_cast<int>(port_));
    });
}

void HttpServer::stop()
{
    server_.stop();
    if (server_thread_.joinable())
        server_thread_.join();
}
