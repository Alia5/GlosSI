#pragma once
#include "HttpServer.h"
#include "../common/Settings.h"
#include "../common/steam_util.h"

namespace CHTE {

inline void addEndpoints()
{
    HttpServer::AddEndpoint(
        {"/settings",
         HttpServer::Method::GET,
         [](const httplib::Request& req, httplib::Response& res) {
             res.set_content(Settings::toJson().dump(), "text/json");
         }});

    HttpServer::AddEndpoint(
        {"/steam_settings",
         HttpServer::Method::GET,
         [](const httplib::Request& req, httplib::Response& res) {
             res.set_content(util::steam::getSteamConfig().dump(4), "text/json");
         }});
};

} // namespace CHTE
