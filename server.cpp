#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include "pthread.h"
#include <vector>
#include <string.h>
#include <mutex>
#include <arpa/inet.h>
#include <fstream>
#include <chrono>
#include <semaphore>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <thread>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdio.h>
#include "utils.cpp"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define BUFFER_SIZE 10000
using namespace std;
// =========================================================================================== //
// Server Body
// =========================================================================================== //

json search_json(string id, string id_field, string sub_path, json input)
{
    for (auto v : input.at(sub_path))
    {
        if (v.at(id_field) == id)
        {
            return v;
        }
    }
    return nullptr;
}

json update_json(string id, string id_field, string sub_path, json input, json new_val)
{
    json new_input = input;
    for (auto &v : new_input.at(sub_path))
    {
        if (v.at(id_field) == id)
        {
            v = new_val;
        }
    }
    return new_input;
}
class Server
{
public:
    int port;

    pthread_mutex_t data_lock;
    Date *cur;
    struct sockaddr_in address;
    int server_fd;
    int server_shutdown();
    int server_send(int client_fd, json jobject);
    int server_accept(int addrlen, struct sockaddr_in address);
    int server_disconnect(int client_fd);
    json server_receive(int client_fd);
    void run();

    Server(string config_location, string users_location, string rooms_location);

    string serialize(json request);
    json deserialize(string resposne);

    vector<int> client_fds;
    bool is_user_admin(string username); // TODO Complete ,
    map<int, char *> client_buffers;
    bool authenticate(json request);

    json get_users() { return this->users; }
    json get_config()
    {
        pthread_mutex_lock(&this->data_lock);
        return this->config;
        pthread_mutex_unlock(&this->data_lock);
    }
    json get_rooms()
    {
        pthread_mutex_lock(&this->data_lock);
        return this->rooms;
        pthread_mutex_unlock(&this->data_lock);
    }
    void set_rooms(json new_rooms)
    {
        pthread_mutex_lock(&this->data_lock);
        this->rooms = new_rooms;
        pthread_mutex_unlock(&this->data_lock);
        ofstream rooms_f(this->rooms_file_location);
        rooms_f << this->rooms.dump(4);
        // persist data in the disk after each update
    }
    void set_configs(json new_config)
    {
        pthread_mutex_lock(&this->data_lock);
        this->config = new_config;
        pthread_mutex_unlock(&this->data_lock);
    }
    void set_users(json new_users)
    {
        pthread_mutex_lock(&this->data_lock);
        this->users = new_users;
        pthread_mutex_unlock(&this->data_lock);
        ofstream users_f(this->users_file_location);
        users_f << this->users.dump(4);
        // persist data in the disk after each update
    }

private:
    string users_file_location;
    string config_file_location;
    string rooms_file_location;
    json users;
    json config;
    json rooms;
};
// =========================================================================================== //
//  Serialization / Deserialization
// =========================================================================================== //

json Server::deserialize(string json)
{
    return json::parse(json);
}

string Server::serialize(json jobject)
{
    return jobject.dump();
}
// =========================================================================================== //
// Auxilariy
// =========================================================================================== //
bool Server::is_user_admin(string username)
{
    return search_json(username, "user", "users", this->get_users()).at("admin") == "true";
}

bool Server::authenticate(json request)
{
    for (auto v : this->users.at("users"))
    {
        if (v.at("user") == request.at("payload").at("username") && v.at("password") == request.at("payload").at("password"))
        {
            return true;
        }
    }
    return false;
}
// =========================================================================================== //
// Responses
// =========================================================================================== //
json pass_day_response(json request, Server *serv)
{
    json response;
    string request_username = request.at("payload").at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    if (target_user != nullptr)
    {
        if (target_user.at("admin") == "true")
        {
            int days = request.at("payload").at("days");
            cout << serv->cur->day << serv->cur->year << serv->cur->month;
            pthread_mutex_lock(&serv->data_lock);
            serv->cur->move(days);
            pthread_mutex_unlock(&serv->data_lock);

            response["code"] = 312;
            response["message"] = "Days passed succesfuly";
            response["current_date"] = serv->cur->get_string();
            cout << serv->cur->day << serv->cur->year << serv->cur->month;
            cout << "Current Server Time Changed " << serv->cur->get_string();
            // TODO PROBABLY HAVE TO UPDATE SERVER STATE HERE
            // SET MUTEX FOR CHANGING server time
        }
        else
        {
            response["code"] = 403;
            response["message"] = "You are not Admin";
        }
    }
    else
    {
        response["code"] = 403;
        response["message"] = "You are not logged in";
    }
    return response;
}

json logout_repsonse(json request, Server *serv)
{
    json response;
    string username = request.at("payload").at("username");
    json target_user = search_json(username, "user", "users", serv->get_users());
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            response["code"] = 230;
            response["message"] = "Logged out succesfully";
            json target_user = search_json(username, "user", "users", serv->get_users());
            target_user["logged_in"] = false;
            json updated_json = update_json(username, "user", "users", serv->get_users(), target_user);
            serv->set_users(updated_json);
        }
    }
    else
    {
        response["code"] = 410;
        response["message"] = "Log out failed ,not logged in!";
    }
    return response;
}

json login_response(json request, Server *serv)
{
    json response;
    bool authenticaiton_result = serv->authenticate(request);
    if (authenticaiton_result)
    {
        if (search_json(request.at("payload").at("username"), "user", "users", serv->get_users()).at("logged_in") == false)
        {
            response["code"] = 230;
            response["username"] = request.at("payload").at("username");
            response["message"] = "Logged in succesfully";
            json target_user = search_json(request.at("payload").at("username"), "user", "users", serv->get_users());
            target_user["logged_in"] = true;
            json updated_json = update_json(request.at("payload").at("username"), "user", "users", serv->get_users(), target_user);
            serv->set_users(updated_json);
        }
        else
        {
            response["code"] = 430;
            response["message"] = "Already Logged in ";
        }
    }
    else
    {
        response["code"] = 430;
        response["message"] = "Log in failed";
    }
    return response;
}

json view_user_information_response(json request, Server *serv)
{
    json response;
    string request_username = request.at("payload").at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    if (target_user != nullptr)
    {
        if (target_user["logged_in"] == true)
        {
            response["code"] = 230;
            response["user"] = target_user;
            response["message"] = "User information Retrived succesfully";
        }
        else
        {
            response["code"] = 430;
            response["message"] = "You are not logged in ";
        }
    }
    else
    {
        response["code"] = 404;
        response["message"] = "Not logged in or user not found";
    }
    return response;
}

void remove_password_from_users(json &users)
{
    for (auto &v : users.at("users"))
    {
        v.erase("password");
    }
}

json all_users_response(json request, Server *serv)
{
    json response;
    string request_username = request.at("payload").at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    json user_backup = serv->get_users();
    remove_password_from_users(user_backup);
    if (target_user != nullptr)
    {
        if (target_user["logged_in"] == true)
        {
            if (target_user["admin"] == "true")
            {
                response["code"] = 230;
                response["users"] = user_backup;
                response["message"] = "User information Retrived succesfully";
            }
            else
            {
                response["code"] = 430;
                response["message"] = "You are not an admin and cannot view all users ";
            }
        }
        else
        {
            response["code"] = 430;
            response["message"] = "You are not logged in ";
        }
    }
    else
    {
        response["code"] = 404;
        response["message"] = "Not logged in or user not found";
    }
    return response;
}

json signup_check_response(json request, Server *serv)
{
    json response;
    if (search_json(request.at("payload").at("username"), "user", "users", serv->get_users()) == nullptr)
    {
        response["message"] = "Username is unique!";
        response["code"] = 313;
    }
    else
    {
        response["message"] = "Username exists , try another one";
        response["code"] = 451;
    }
    return response;
}

json signup_response(json request, Server *serv)
{
    int id = serv->get_users().count("users") + 1;
    json new_user;
    json request_payload = request.at("payload");
    string id_ = to_string(id);
    new_user["id"] = id_;
    new_user["address"] = request_payload.at("address");
    new_user["phoneNumber"] = request_payload.at("phone");
    new_user["password"] = request_payload.at("password");
    new_user["user"] = request_payload.at("username");
    new_user["logged_in"] = false;
    json updated_json = serv->get_users();
    json inner = updated_json.at("users");
    inner.push_back(new_user);
    updated_json["users"] = inner;
    serv->set_users(updated_json);
    json response;
    response["code"] = 231;
    return response;
}
json generate_response_fx(json request, Server *serv)
{
    string request_type = request.at("type");
    json response;
    if (request_type == "login")
    {
        response = login_response(request, serv);
        if (response.at("code") == 230)
        {
            json target_user = search_json(request.at("payload").at("username"), "user", "users", serv->get_users());
            target_user["logged_in"] = true;
        }
    }
    else if (request_type == "signup_check")
    {
        response = signup_check_response(request, serv);
    }
    else if (request_type == "signup")
    {
        response = signup_response(request, serv);
    }
    else if (request_type == "logout")
    {
        response = logout_repsonse(request, serv);
    }
    else if (request_type == "pass_day")
    {
        response = pass_day_response(request, serv);
    }
    else if (request_type == "view_user_information")
    {
        response = view_user_information_response(request, serv);
    }
    else if (request_type == "all_users")
    {
        response = all_users_response(request, serv);
    }
    return response;
}
void handle_client(Server *serv, int client_fd)
{
    cout << "Started Clien