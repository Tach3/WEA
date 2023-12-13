#pragma once
#include "sha256.h"
#include "ExpirationCache.h"
#include <regex>
using namespace std;
using json = nlohmann::json;

#define USERSJ "users.json"
#define USERNAME 1
#define PASSWORD 2
#define REPOJ "_repo.json"
#define NULL_VAL "NULL"
#define SESSION_TIME 900

//parses json file
json parseJson(std::string file) {
    ifstream json_file(file);
    json data;
    json_file >> data;
    return data;
}
//bool to check authentication
bool authenticate(const string &username, string &password, json &data) {
    SHA256 sha256;
    password = sha256(password);
    for (const auto& user : data) {
        if (user["username"] == username && user["password"] == password) {
            return true; // Authentication successful
        }
    }
    return false;
}
//helper function to separate and decode base64 username:password
string separateAuth(crow::request req, int returnable) {
    string myauth = req.get_header_value("Authorization");
    string mycreds = myauth.substr(6);
    string d_mycreds = crow::utility::base64decode(mycreds, mycreds.size());
    size_t found = d_mycreds.find(':');
    switch (returnable) {
    case USERNAME:
        return d_mycreds.substr(0, found);
    case PASSWORD:
        return d_mycreds.substr(found + 1);
    default:
        return "wrong hash lmao";
    }
}
//authenticates user based on username and password
bool authentication(crow::request req) {
    std::string username = separateAuth(req, USERNAME);
    std::string password = separateAuth(req, PASSWORD);
    json data = parseJson(USERSJ);
    if (authenticate(username, password, data)) {
        return true;
    }
    else {
        return false;
    }
}
//generates session id using sha256
string generateSessionId() {
    SHA256 sha256;
    srand(time(NULL));  // Seed the random number generator with the current time
    string random_number = to_string(rand() % 9000 + 1000);  // Generate a random number between 1000 and 9999
    string sessionId = sha256(random_number);
    return sessionId;
}
//gets username from cookie
string getUserFromCookie(crow::CookieParser::context& ctx) {
    string cookie_id = ctx.get_cookie("session");
    size_t found = cookie_id.find('=');
    string user_id = cookie_id.substr(found + 1);
    return user_id;
}
//gets session id from cookie
string getSessionFromCookie(crow::CookieParser::context& ctx) {
    string cookie_id = ctx.get_cookie("session");
    size_t found = cookie_id.find('=');
    string session_id = cookie_id.substr(0, found);
    return session_id;
}
//checks from cookie if session is valid
bool checkCookie(crow::CookieParser::context &ctx, ExpirationCache<std::string, std::string, SESSION_TIME>& session) {
    if (ctx.get_cookie("session").empty()) {
        return true;
    }
    string cookie_id = ctx.get_cookie("session");
    size_t found = cookie_id.find('=');
    string session_id = cookie_id.substr(0, found);
    string user_id = cookie_id.substr(found + 1);
    try
    {
        if (user_id == session.Get(session_id)) {
            return false;
        }
    }
    catch (std::out_of_range& e)
    {
        return true;
    }
    return true;
}

//updates task from complete to incomplete and vice versa
void updateTask(const string& username, string& todo_name) {
    boost::replace_all(todo_name, "%20", " ");
    // Read JSON data from file
    string filename = username + REPOJ;
    ifstream ifs(filename);
    json data_json;
    ifs >> data_json;

    // Toggle the completed status for an item with a specific name
    for (auto& item : data_json["repo"]) {
        if (item["name"] == todo_name) {
            // Toggle the completed status
            item["completed"] = !item["completed"];

            // Write the updated JSON data back to the file
            ofstream ofs(filename);
            ofs << data_json.dump(4) << endl;
        }
    }
}
//deletes task
void deleteTask(const string& username, string& todo_name) {
    boost::replace_all(todo_name, "%20", " ");
    // Read JSON data from file
    string filename = username + REPOJ;
    ifstream ifs(filename);
    json data_json;
    ifs >> data_json;
    auto& repo = data_json["repo"];
    auto it = std::find_if(repo.begin(), repo.end(), [&todo_name](const auto& item) {
        return item["name"] == todo_name;
    });

    repo.erase(it);

    // Write the updated JSON data back to the file
    ofstream ofs(filename);

    ofs << data_json.dump(4) << endl;
}
//adds a task
void addTask(const std::string& username, std::string& todo_name) {
    std::regex pattern("[^a-zA-Z0-9]");
    string sanitized_name = std::regex_replace(todo_name, pattern, " ");
    json task = {
        {"completed", false},
        {"name", sanitized_name}
    };

    string filename = username + REPOJ;

    // Read existing JSON data
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        // If the file doesn't exist, create a new JSON structure
        json data_json = {
            {"repo", json::array()}
        };
        data_json["repo"].push_back(task);

        // Write the new JSON data to the file
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << data_json.dump(4) << endl;
            cout << "Task added successfully." << endl;
        }
        else {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
        }
        return;
    }

    // File exists, so update the existing JSON data
    json data_json;
    ifs >> data_json;
    ifs.close();  // Close the ifstream

    // Add the new task
    data_json["repo"].push_back(task);

    // Write the updated JSON back to the file
    std::ofstream ofs(filename);
    if (ofs.is_open()) {
        ofs << data_json.dump(4) << std::endl;
        cout << "Task added successfully." << endl;
    }
    else {
        cerr << "Error opening file for writing: " << filename << endl;
    }
}

