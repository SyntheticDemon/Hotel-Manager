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
#include "logger.cpp"

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

int search_json_array(string id, string id_field, string sub_path, json input)
{
    int x = 0;
    for (auto v : input.at(sub_path))
    {
        if (v.at(id_field) == id)
        {
            return x;
        }
        x += 1;
    }
    return -1;
}
json search_json_reference(string id, string id_field, string sub_path, json input)
{
    for (auto &v : input.at(sub_path))
    {
        if (v.at(id_field) == id)
        {
            cout << v;
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
    Logger *logger;

    int port;

    pthread_mutex_t user_data_lock;
    pthread_mutex_t room_data_lock;

    Date *cur;
    struct sockaddr_in address;
    int server_fd;
    int server_shutdown();
    int server_send(int client_fd, json jobject);
    int server_accept(int addrlen, struct sockaddr_in address);
    int server_disconnect(int client_fd);
    json server_receive(int client_fd);
    void run();

    Server(string config_location, string users_location, string rooms_location, string log_file);

    string serialize(json request);
    json deserialize(string resposne);

    bool is_user_admin(string username); // TODO Complete ,
    map<int, char *> client_buffers;
    bool authenticate(json request);
    int overlap_occuptations(string start, string end, json target_room);
    bool invalid_date(string start, string end);
    void update_reserves_with_time();
    json get_users()
    {
        return this->users;
    }
    json get_config()
    {
        return this->config;
    }
    json get_rooms()
    {
        return this->rooms;
    }
    void set_rooms(json new_rooms)
    {
        pthread_mutex_lock(&this->room_data_lock);
        this->rooms = new_rooms;
        pthread_mutex_unlock(&this->room_data_lock);
        ofstream rooms_f(this->rooms_file_location);
        rooms_f << this->rooms.dump(4);
        // persist data in the disk after each update
    }
    void set_configs(json new_config)
    {
        this->config = new_config;
    }
    void set_users(json new_users)
    {
        pthread_mutex_lock(&this->user_data_lock);
        this->users = new_users;
        pthread_mutex_unlock(&this->user_data_lock);
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

void Server::update_reserves_with_time()
{

    json rooms_backup = this->get_rooms();
    json rooms = rooms_backup.at("rooms");
    cout << rooms.dump(4);
    for (auto &v : rooms_backup.at("rooms"))
    {
        json reservations = v.at("users");
        if (reservations.size() != 0)
        {
            json new_reservations = json::array();
            for (auto v_2 : v.at("users"))
            {
                string ckout = v_2.at("checkout_date");
                Date checkout = Date(ckout);
                if (smaller(checkout, (*this->cur)))
                {
                    cout << "Date was smaller therefore stays";
                    new_reservations.push_back(v_2);
                }
            }
            v["users"] = new_reservations;
        }
    }
    json new_rooms = rooms_backup;
    cout << new_rooms.dump(4);
    this->set_rooms(new_rooms);
}
int Server::overlap_occuptations(string start, string end, json target_room)
{
    vector<int> overlap_occupationss = {0};
    Date start_date(start);
    Date end_date(end);
    for (auto v : target_room.at("users"))
    {
        Date reserve_date = Date(v.at("check_in_date"));
        Date checkout_date = Date(v.at("checkout_date"));
        if (have_overlap(start_date, end_date, reserve_date, checkout_date))
        {
            overlap_occupationss.push_back(v.at("bed_count"));
        }
    }
    return *max_element(overlap_occupationss.begin(), overlap_occupationss.end());
}

bool Server::invalid_date(string start, string end)
{
    Date start_date(start);
    Date end_date(end);
    bool result =
        smaller(start_date, end_date) || smaller((*this->cur), end) || smaller((*this->cur), start);
    cout << getTime() << "Checking Period Start : " << start << " End : " << end << " Result " << result << " Server Time " << (*this->cur).get_string() << endl;
    cout << getTime() << "First " << smaller(start_date, end_date) << " Second "
         << smaller(end, (*this->cur)) << " Third " << smaller(start, (*this->cur)) << endl;
    return result;
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
            cout << getTime() << serv->cur->day << serv->cur->year << serv->cur->month;
            serv->cur->move(days);

            response["code"] = 312;
            response["message"] = "Days passed succesfuly";
            response["current_date"] = serv->cur->get_string();
            cout << getTime() << serv->cur->day << serv->cur->year << serv->cur->month;
            cout << getTime() << "Current Server Time Changed " << serv->cur->get_string();
            serv->update_reserves_with_time();
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
            response["username"] = request.at("payload").at("username");
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

json view_room_information_response(json request, Server *serv)
{
    json response;
    string request_username = request.at("payload").at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    json rooms_backup = serv->get_rooms();
    if (target_user != nullptr)
    {
        if (target_user["logged_in"] == true)
        {
            response["code"] = 230;
            response["rooms"] = rooms_backup;
            response["message"] = "Room Information Retrived succesfully";
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
json edit_information_response(json request, Server *serv)
{
    json response;
    json payload = request.at("payload");
    json target_user = search_json(payload.at("username"), "user", "users", serv->get_users());
    if (target_user.at("logged_in") == true)
    {
        response["code"] = 312;
        response["username"] = request.at("payload").at("username");
        response["message"] = "Information edited succesfully";
        target_user["password"] = payload.at("new_password");
        target_user["phone"] = payload.at("phone");
        target_user["address"] = payload.at("address");
        json updated_json = update_json(request.at("payload").at("username"), "user", "users", serv->get_users(), target_user);
        serv->set_users(updated_json);
    }
    else
    {
        response["code"] = 503;
        response["message"] = "You are not logged in  ";
        response["username"] = request.at("payload").at("username");
    }
    return response;
}

json book_room_response(json request, Server *serv)
{

    json response;
    json payload = request.at("payload");
    string start = payload.at("check_in_date");
    string end = payload.at("checkout_date");
    string request_username = payload.at("username");
    json target_user = search_json_reference(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json_reference(target_room_id, "number", "rooms", serv->get_rooms());
    if (target_room == nullptr)
    {
        response["message"] = "Room was not found";
        response["code"] = 101;
        return response;
    }
    int overlap_occu = serv->overlap_occuptations(start, end, target_room);
    bool invalid_period = serv->invalid_date(start, end);
    if (invalid_period)
    {
        response["code"] = 503;
        response["message"] = "Invalid input";
        response["description"] = "Server time is " + serv->cur->get_string();
        return response;
    }
    int bed_count = atoi(string(payload.at("bed_count")).c_str());
    int room_per_person_price = target_room.at("price");
    int room_cost = room_per_person_price * bed_count;
    json rooms_backup = serv->get_rooms();
    json users_backup = serv->get_users();

    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            if (target_user.at("admin") == "true")
            {
                response["code"] = 403;
                response["message"] = "You an Admin and cannot book rooms";
            }
            else
            {
                int balance = atoi(string(target_user.at("purse")).c_str());
                cout << getTime() << "User balance was " << balance << " Room cost was " << room_cost << endl;
                if (balance < room_cost)
                {
                    response["code"] = 108;
                    response["message"] = "Not enough balance";
                    return response;
                }
                else
                {
                    int max_cap = target_room.at("max_capacity");
                    cout << getTime() << " Room max capacity " << max_cap << " Overlap max " << overlap_occu << endl;

                    if (bed_count > max_cap - overlap_occu)
                    {
                        response["code"] = 109;
                        response["message"] = "The room's max capacity in the requested period is less than request's bed count";
                        return response;
                    }
                    else
                    {
                        response["code"] = 230;
                        response["rooms"] = rooms_backup;
                        target_user["purse"] = to_string(balance - room_cost);
                        json reservation;
                        reservation["bed_count"] = bed_count;
                        reservation["id"] = target_user.at("id");
                        reservation["check_in_date"] = start;
                        reservation["checkout_date"] = end;
                        target_room.at("users")
                            .push_back(reservation);
                        json new_rooms = update_json(target_room_id, "number", "rooms", rooms_backup, target_room);
                        json new_users = update_json(request_username, "user", "users", users_backup, target_user);
                        serv->set_users(new_users);
                        serv->set_rooms(new_rooms);
                        response["message"] = "Booked room and updated balance succesfully";
                        return response;
                    }
                }
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
    int id = serv->get_users().at("users").size() + 1;
    json new_user;
    json request_payload = request.at("payload");
    string id_ = to_string(id);
    new_user["id"] = id_;
    new_user["address"] = request_payload.at("address");
    new_user["phoneNumber"] = request_payload.at("phone");
    new_user["password"] = request_payload.at("password");
    new_user["user"] = request_payload.at("username");
    new_user["logged_in"] = false;
    new_user["admin"] = false;
    new_user["purse"] = request_payload.at("purse");
    json updated_json = serv->get_users();
    json inner = updated_json.at("users");
    inner.push_back(new_user);
    updated_json["users"] = inner;
    serv->set_users(updated_json);
    json response;
    response["code"] = 231;
    return response;
}

json modify_room_response(json request, Server *serv)
{

    json response;
    json payload = request.at("payload");
    string room_id = payload.at("id");
    string price = payload.at("price");
    string max_capacity = payload.at("max_capacity");
    string request_username = payload.at("username");
    json target_user = search_json_reference(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json_reference(target_room_id, "number", "rooms", serv->get_rooms());
    if (target_room == nullptr)
    {
        response["message"] = "Room was not found";
        response["code"] = 101;
        return response;
    }
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            if (target_user.at("admin") == "false")
            {
                response["code"] = 403;
                response["message"] = "You not an Admin and don't have access to rooms";
            }
            else
            {
                json rooms_backup = serv->get_rooms();
                json new_room;
                int max_capacity_int = atoi(max_capacity.c_str());
                new_room["capacity"] = 0;
                new_room["max_capacity"] = max_capacity;
                new_room["status"] = 0;
                new_room["users"] = json::array();
                new_room["number"] = room_id;
                rooms_backup = update_json(target_room_id, "number", "rooms", rooms_backup, new_room);
                serv->set_rooms(rooms_backup);
                response["code"] = 104;
                response["message"] = "Room modified succesfully";
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

json remove_room_response(json request, Server *serv)
{
    json response;
    json payload = request.at("payload");
    string room_id = payload.at("id");
    string request_username = payload.at("username");
    json target_user = search_json_reference(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json_reference(target_room_id, "number", "rooms", serv->get_rooms());
    if (target_room != nullptr)
    {
        response["message"] = "Room id is occupied";
        response["code"] = 111;
        return response;
    }
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            if (target_user.at("admin") == "false")
            {
                response["code"] = 403;
                response["message"] = "You not an Admin and don't have access to rooms";
            }
            else
            {
                if (target_room.at("status"))
                {

                    json rooms_backup = serv->get_rooms();
                    int room_index = search_json_array(target_room_id, "number", "rooms", rooms_backup);
                    rooms_backup.at("rooms").erase(room_index);
                    serv->set_rooms(rooms_backup);
                    response["code"] = 104;
                    response["message"] = "New room created";
                }
                else
                {
                    response["code"] = 109;
                    response["message"] = "Room is full and uneditble";
                }
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

json add_room_response(json request, Server *serv)
{

    json response;
    json payload = request.at("payload");
    string room_id = payload.at("id");
    string price = payload.at("price");
    string max_capacity = payload.at("max_capacity");
    string request_username = payload.at("username");
    json target_user = search_json_reference(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json_reference(target_room_id, "number", "rooms", serv->get_rooms());
    if (target_room != nullptr)
    {
        response["message"] = "Room id is occupied";
        response["code"] = 111;
        return response;
    }
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            if (target_user.at("admin") == "false")
            {
                response["code"] = 403;
                response["message"] = "You not an Admin and don't have access to rooms";
            }
            else
            {
                json rooms_backup = serv->get_rooms();
                json new_room;
                int max_capacity_int = atoi(max_capacity.c_str());
                new_room["capacity"] = 0;
                new_room["max_capacity"] = max_capacity;
                new_room["status"] = 0;
                new_room["users"] = json::array();
                new_room["number"] = room_id;
                rooms_backup.at("rooms").push_back(new_room);
                serv->set_rooms(rooms_backup);
                response["code"] = 104;
                response["message"] = "New room created";
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
    else if (request_type == "view_room_information")
    {
        response = view_room_information_response(request, serv);
    }
    else if (request_type == "book_room")
    {

        response = book_room_response(request, serv);
    }
    else if (request_type == "edit_information")
    {

        response = edit_information_response(request, serv);
    }
    else if (request_type == "remove_room")
    {
        response = remove_room_response(request, serv);
    }
    else if (request_type == "modify")
    {
        response = modify_room_response(request, serv);
    }
    else if (request_type == "add")
    {
        response = add_room_response(request, serv);
    }
    return response;
}
void handle_client(Server *serv, int client_fd)
{
    cout << getTime() << "Started Client Thread " << client_fd << endl;

    while (true)
    {
        char new_client_buffer[BUFFER_SIZE] = {0};
        if (read(client_fd, new_client_buffer, BUFFER_SIZE) == 0)
        {

            cout << getTime() << "Client FD did not return valid request" << endl;
            serv->server_disconnect(client_fd);
            break;
        }
        else
        {

            std::string cpp_message(new_client_buffer);
            json request = serv->deserialize(cpp_message);
            cout << getTime() << "Received Request : " << request << endl;
            json response = generate_response_fx(request, serv);
            cout << getTime() << "Sending Response : " << response << endl;
            serv->server_send(client_fd, response);
        }
    }
};

int Server::server_accept(int addrlen, struct sockaddr_in address)
{
    int new_socket;
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("Failure in Accept");
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << getTime() << "Allocating Resources for this consumer (A buffer and A thread) " << new_socket << endl;
        thread th1(handle_client, this, new_socket);
        th1.detach();
        // new_client_buffer, new_client_fd
        // );
    }
    return new_socket;
}

int Server::server_shutdown()
{
    return shutdown(this->server_fd, SHUT_RDWR);
}
// =========================================================================================== //
//  Links
// =========================================================================================== //

int Server::server_send(int client_fd, json jobject)
{

    string message = this->serialize(jobject);
    const char *m = message.c_str();
    return send(client_fd, m, strlen(m), 0);
}

json Server::server_receive(int client_fd)
{
    char temp_buffer[BUFFER_SIZE] = {0};

    if (read(client_fd, temp_buffer, BUFFER_SIZE))
    {
        perror("Could not read from client FD");
    }
    std::string cpp_message(temp_buffer);
    json final = this->deserialize(cpp_message);
    return final;
}

void Server::run()
{

    string initial_date;
    cout << getTime() << "Enter initial date \n";
    cin >> initial_date;
    cout << getTime() << "Updating reserves with time \n";
    Date curr = Date(initial_date);
    this->cur = &curr;

    update_reserves_with_time();
    while (true)
        this->Server::server_accept(sizeof(address), address);
}

int Server::server_disconnect(int client_fd)
{

    return close(client_fd);
}
// =========================================================================================== //
//  Consturctor
// =========================================================================================== //
Server::Server(string config_location, string users_location, string rooms_location, string log_file)
{
    int server_fd;
    Logger l;
    this->logger = &l;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    this->config = read_json_f(config_location);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << getTime() << "Opening socket failed" << endl;
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        cout << getTime() << "Set socket options failed" << endl;
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->config.at("commandChannelPort"));
    // Forcefully attaching socket to port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    this->rooms = read_json_f(rooms_location);
    json users = read_json_f(users_location);
    for (auto &v : users.at("users"))
    {
        v["logged_in"] = false;
    }
    int peers = this->users.count("users");
    if (listen(server_fd, peers) < 0)
    {
        cout << getTime() << "Listen Failed" << endl;
        exit(EXIT_FAILURE);
    }
    this->users = users;
    this->port = port;
    this->address = address;
    this->config_file_location = config_location;
    this->rooms_file_location = rooms_location;
    this->users_file_location = users_location;

    cout << getTime() << "Initialized server with current state" << endl;
    cout << getTime() << "Users :" << this->users.dump(4) << endl;
    cout << getTime() << "Rooms:" << this->rooms.dump(4) << endl;
    cout << getTime() << "Config :" << this->users.dump(4) << endl;

    this->server_fd = server_fd;
    // read files
}
// =========================================================================================== //
// =========================================================================================== //

int main(int argc, char const *argv[])
{
    // setup server logging
    string log_file = string(argv[1]);
    string users_location = "jsons/users.json";
    string rooms_location = "jsons/rooms.json";
    string configs_location = "jsons/config.json";
    // while (true)
    // {
    //     string initial_input;
    //     cin >> initial_input;
    //     vector<string> inp = split(initial_input, ' ');
    //     if (inp[0] == "setTime")
    //     {
    //         cout << getTime() << inp[0];
    //         initial_date = inp[1];
    //         break;
    //     }
    // }
    Server new_server = Server(configs_location, users_location, rooms_location, log_file);

    new_server.run();

    return 0;
}