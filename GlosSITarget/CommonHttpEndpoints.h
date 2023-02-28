#pragma once
#include "HttpServer.h"
#include "../common/Settings.h"
#include "../common/steam_util.h"

namespace CHTE {

inline void addEndpoints()
{

        HttpServer::AddEndpoint(
        {"/running",
         HttpServer::Method::GET,
         [](const httplib::Request& req, httplib::Response& res) {
             // TODO: extend this when "passive" running of global mods is implemented
             res.set_content(nlohmann::json{{"state", nlohmann::json{{"running", true}}}}.dump(), "text/json");
         }});

    HttpServer::AddEndpoint(
        {"/settings",
         HttpServer::Method::GET,
         [](const httplib::Request& req, httplib::Response& res) {
             res.set_content(Settings::toJson().dump(), "text/json");
         },
             "json"});

    HttpServer::AddEndpoint(
        {"/steam_settings",
         HttpServer::Method::GET,
         [](const httplib::Request& req, httplib::Response& res) {
             res.set_content(util::steam::getSteamConfig().dump(4), "text/json");
         },
         "json"});

    HttpServer::AddEndpoint({
        "/log",
        HttpServer::Method::POST,
        [](const httplib::Request& req, httplib::Response& res) {
            struct LogEntry {
                std::string level;
                std::string message;
            };
            auto entry = LogEntry{};
            try {
                const nlohmann::json postbody = nlohmann::json::parse(req.body);
                entry.level = postbody.at("level");
                entry.message = postbody.at("message");
            }
            catch (std::exception& e) {
                res.status = 401;
                res.set_content(nlohmann::json{
                                    {"code", 401},
                                    {"name", "Bad Request"},
                                    {"message", e.what()},
                                }
                                    .dump(),
                                "text/json");
            }
            if (entry.level == "info") {
                spdlog::info("GlosSITweaks: {}", entry.message);
            }
            else if (entry.level == "warn") {
                spdlog::warn("GlosSITweaks: {}", entry.message);
            }
            else if (entry.level == "error") {
                spdlog::error("GlosSITweaks: {}", entry.message);
            }
            else if (entry.level == "debug") {
                spdlog::debug("GlosSITweaks: {}", entry.message);
            }
            else {
                spdlog::trace("GlosSITweaks: {}", entry.message);
            }
        },

    });
    
};

} // namespace CHTE
