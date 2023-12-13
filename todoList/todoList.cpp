#include <iostream>
#include <string>
#include "crow_all.h"
#include "json.hpp"
#include "myFunctions.h"
#include "sha256.h"
#include "sha256.cpp"
#include <random>
#include "ExpirationCache.h"



using namespace std;
using json = nlohmann::json;


enum class SameSitePolicy
{
    Strict,
    Lax,
    None
};

crow::response redirect(string location)
{
    crow::response res;
    res.redirect(location);
    return res;
}

int main()
{
    crow::App<crow::CookieParser, crow::CORSHandler> app;
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors
        .global()
        .methods("POST"_method, "GET"_method, "OPTIONS"_method)
        .origin("https://todoapp-a7a9f5a0e861.herokuapp.com")
        .allow_credentials()
        .headers("*", "Content-Type", "Authorization");

    ExpirationCache<string, string, SESSION_TIME> session;

    CROW_ROUTE(app, "/dashboard")
        ([&](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        if (checkCookie(ctx, session)) {
            auto page = crow::mustache::load("login.html");
            return redirect("/");
        }
        string username = getUserFromCookie(ctx);
        json data = parseJson(username + REPOJ);
        auto rv = crow::json::load(data.dump());
        crow::mustache::context x{ rv };
        x["username"] = username;
        auto page = crow::mustache::load("dashboard.html");
        return crow::response(page.render(x));
            });

    CROW_ROUTE(app, "/login").methods("POST"_method)
        ([&](const crow::request& req) {
        if (authentication(req)) {
            string session_id = generateSessionId();
            string user_id = separateAuth(req,USERNAME);
            session.Put(session_id, user_id);
            string cookie_value = session_id + "=" + user_id;
            auto& ctx = app.get_context<crow::CookieParser>(req);

            // set a cookie
            ctx.set_cookie("session", cookie_value)
                .path("/")
                .secure()
                .max_age(SESSION_TIME)
                .same_site(crow::CookieParser::Cookie::SameSitePolicy::None)
                .httponly();
            return crow::response(200);
        }
        return crow::response(401);
            });

    CROW_ROUTE(app, "/")
        ([&](const crow::request& req) {
        auto page = crow::mustache::load("login.html");
        return page.render();
            });

    CROW_ROUTE(app, "/script")
        ([&](const crow::request& req) {
        crow::response res;
        res.set_static_file_info("templates/login.js");
        return res;
            });

    CROW_ROUTE(app, "/update/<string>")
        ([&](const crow::request& req, string todo_name) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        if (checkCookie(ctx, session)) {
            auto page = crow::mustache::load("login.html");
            return redirect("/");
        }
        string username = getUserFromCookie(ctx);
        updateTask(username, todo_name);
        auto page = crow::mustache::load("dashboard.html");
        return redirect("/dashboard");
            });

    CROW_ROUTE(app, "/delete/<string>")
        ([&](const crow::request& req, string todo_name) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        if (checkCookie(ctx, session)) {
            auto page = crow::mustache::load("login.html");
            return redirect("/");
        }
        string username = getUserFromCookie(ctx);
        deleteTask(username, todo_name);
        auto page = crow::mustache::load("dashboard.html");
        return redirect("/dashboard");
            });

    CROW_ROUTE(app, "/add")
        ([&](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        if (checkCookie(ctx, session)) {
            auto page = crow::mustache::load("login.html");
            return redirect("/");
        }
        string username = getUserFromCookie(ctx);
        string todo_name = req.url_params.get("title");
        addTask(username, todo_name);
        auto page = crow::mustache::load("dashboard.html");
        return redirect("/dashboard");
            });

    CROW_ROUTE(app, "/logout")
        ([&](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        if (checkCookie(ctx, session)) {
            auto page = crow::mustache::load("login.html");
            return redirect("/");
        }
        string session_id = getSessionFromCookie(ctx);
        session.Put(session_id, NULL_VAL);
        return redirect("/");
            });

    CROW_ROUTE(app, "/json")
        ([&](const crow::request& req) {
        auto& ctx = app.get_context<crow::CookieParser>(req);
        if (checkCookie(ctx, session)) {
            auto page = crow::mustache::load("login.html");
            return redirect("/");
        }
        string username = getUserFromCookie(ctx);
        json data = parseJson(username + REPOJ);
        return crow::response(data.dump());
            });

    CROW_CATCHALL_ROUTE(app)
    ([](crow::response& res) {
        if (res.code == 404){
            res.body = "This seems to be the wrong url/endpoint.";
        }
        else if (res.code == 405){
            res.body = "The HTTP method does not seem to be correct/allowed.";
        }
        res.end();
        });
    char* port = std::getenv("PORT");
    uint16_t iPort = static_cast<uint16_t>(port != NULL ? stoi(port) : 18080);
    std::cout << "Port: " << iPort << endl;
    app.port(iPort).multithreaded().run();
}



