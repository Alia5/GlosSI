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
#include <utility>

#include <algorithm>

HttpServer::HttpServer(std::function<void()> close) : close_(std::move(close))
{
}

std::string HttpServer::ToString(Method m)
{
    switch (m) {
    case POST:
        return "POST";
    case PATCH:
        return "PATCH";
    case PUT:
        return "PUT";
    default:
        return "GET";
    }
}

void HttpServer::AddEndpoint(const Endpoint&& e)
{
    endpoints_.push_back(e);
}

void HttpServer::run()
{
    auto setCorsHeader = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
    };

    server_.Get("/", [this, &setCorsHeader](const httplib::Request& req, httplib::Response& res) {
        setCorsHeader(res);

        auto content_json = nlohmann::json{
            {"endpoints", nlohmann::json::array()}};

        for (const auto& e : endpoints_) {
            content_json["endpoints"].push_back(
                nlohmann::json{
                    {"path", e.path},
                    {"method", ToString(e.method)},
                    {"response", e.response_hint},
                    {"payload", e.payload_hint},
                });
        }

        content_json["endpoints"].push_back(
            nlohmann::json{
                {"path", "/quit"},
                {"method", "POST"}
            });

        res.set_content(content_json.dump(4),
                        "text/json");
    });

    for (const auto& e : endpoints_) {
        const auto fn = ([this, &e]() -> httplib::Server& (httplib::Server::*)(const std::string&, httplib::Server::Handler) {
            switch (e.method) {
            case POST:
                return &httplib::Server::Post;
            case PUT:
                return &httplib::Server::Put;
            case PATCH:
                return &httplib::Server::Patch;
            default:
                return &httplib::Server::Get;
            }
        })();

        (server_.*fn)(e.path, [this, &e, &setCorsHeader](const httplib::Request& req, httplib::Response& res) {
            setCorsHeader(res);
            res.status = 0;
            res.content_length_ = 0;
            try {
                e.handler(req, res);
            }
            catch (std::exception& err) {
                spdlog::error("Exception in http handler: {}", err.what());
                res.status = res.status == 0 ? 500 : res.status;
                if (res.content_length_ == 0) {
                    res.set_content(nlohmann::json{
                                        {"code", res.status},
                                        {"name", "HandlerError"},
                                        {"message", err.what()},
                                    }
                                        .dump(),
                                    "text/json");
                }
            }
            catch (...) {
                res.status = 500;
                res.set_content(nlohmann::json{
                                    {"code", res.status},
                                    {"name", "Internal Server Error"},
                                    {"message", "Unknown Error"},
                                }
                                    .dump(),
                                "text/json");
            }
        });
    }

    server_.Post("/quit", [this, &setCorsHeader](const httplib::Request& req, httplib::Response& res) {
        setCorsHeader(res);
        close_();
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
