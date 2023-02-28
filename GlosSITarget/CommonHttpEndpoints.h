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
    
};

} // namespace CHTE
