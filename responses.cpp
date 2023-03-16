#ifndef RESPONSECPP
#define RESPONESCPP
#include "server.h"
#include "json_utils.h"
#include "logger.h"
#include "my_utils.h"
#include "responses.h"
#include <exception>
#include <iostream>
#include <map>
#include <vector>
#include <string.h>
#include <mutex>
#include <arpa/inet.h>
#include <fstream>
// #include <semaphore>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

json logout_response(json request, Server *serv)
{
    json response;
    string username = request.at("payload").at("username");
    json target_user = search_json(username, "user", "users", serv->get_users());
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            response["code"] = 201;
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
            response["code"] = 230;
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
            response["rooms"] = rooms_backup.at("rooms");
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

json view_empty_rooms_response(json request, Server *serv)
{
    json response;
    string request_username = request.at("payload").at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    json rooms_backup = serv->get_rooms();
    json filtered_rooms_backup = json::array();
    if (target_user != nullptr)
    {
        if (target_user["logged_in"] == false)
        {
            response["code"] = 430;
            response["message"] = "You are not logged in ";
            return response;
        }
        else
        {
            for (auto v : rooms_backup.at("rooms"))
            {
                int capacity = serv->get_room_capacity(v);
                if (capacity == v.at("max_capacity"))
                {
                    filtered_rooms_backup.push_back(v);
                }
            }
            response["rooms"] = filtered_rooms_backup;
            response["code"] = 230;
            response["message"] = "Empty rooms retrieved succesfully";
            return response;
        }
    }
    else
    {
        response["code"] = 404;
        response["message"] = "Not logged in or user not found";
        return response;
    }
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
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json(target_room_id, "number", "rooms", serv->get_rooms());
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
                        target_room["capacity"] = serv->get_room_capacity(target_room);
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
        response["code"] = 311;
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
    new_user["id"] = id;
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
    string room_id = payload.at("room_number");
    string new_price = payload.at("new_price");
    string new_max_capacity = payload.at("new_max_capacity");
    string request_username = payload.at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json(target_room_id, "number", "rooms", serv->get_rooms());
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
                int new_max_capacity_int = atoi(new_max_capacity.c_str());
                int new_price_int = atoi(new_price.c_str());
                int target_room_capacity = target_room.at("capacity");
                int target_room_max_capacity = target_room.at("max_capacity");
                if ((target_room_capacity != target_room_max_capacity) && (new_max_capacity_int < target_room_max_capacity))
                {
                    response["code"] = 109;
                    response["message"] = "You cannot reduce the room capacity when it has residents";
                }
                else
                {
                    json new_room = target_room;
                    new_room["price"] = new_price_int;
                    new_room["max_capacity"] = new_max_capacity_int;
                    rooms_backup = update_json(target_room_id, "number", "rooms", serv->get_rooms(), new_room);
                    serv->set_rooms(rooms_backup);
                    response["code"] = 105;
                    response["message"] = "Room modified succesfully";
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

json remove_room_response(json request, Server *serv)
{
    json response;
    json payload = request.at("payload");
    string room_id = payload.at("room_number");
    string request_username = payload.at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json(target_room_id, "number", "rooms", serv->get_rooms());

    if (target_room == nullptr)
    {
        response["message"] = "Room not found";
        response["code"] = 111;
    }
    else if (target_room != nullptr)
    {
        int target_room_capacity = target_room.at("capacity");
        int target_room_max_capacity = target_room.at("max_capacity");
        if (target_room_capacity != target_room_max_capacity)
        {
            response["message"] = "Room id is occupied and you cannot remove it";
            response["code"] = 109;
            return response;
        }
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
                int room_index = search_json_array(target_room_id, "number", "rooms", rooms_backup);
                rooms_backup.at("rooms").erase(room_index);

                serv->set_rooms(rooms_backup);
                response["code"] = 106;
                response["message"] = "Removed Room succesfully";
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

json leave_room_response(json request, Server *serv)
{
    json response;
    json payload = request.at("payload");
    string room_id = payload.at("room_number");
    string request_username = payload.at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    json target_room = search_json(room_id, "number", "rooms", serv->get_rooms());
    if (target_room == nullptr)
    {
        response["message"] = "Room not found";
        response["code"] = 503;
        return response;
    }
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            Date current_serv_time = *(serv->cur);
            json rooms_backup = serv->get_rooms();
            int reserve_index = search_json_array_int_key(target_user.at("id"), "id", "users", target_room);
            if (reserve_index == -1)
            {
                response["code"] = 102;
                response["message"] = "You do not reside in this room";
            }
            else
            {
                json target_room_reservations = target_room.at("users");
                json reservation = target_room_reservations[reserve_index];
                string checkin = reservation.at("check_in_date");
                string checkout = reservation.at("checkout_date");
                bool resides = not(smaller(current_serv_time, Date(checkin))) && smaller(current_serv_time, Date(checkout));
                if (not(resides))
                {
                    response["code"] = 102;
                    response["message"] = "You do not reside in this room at this time";
                }
                else
                {
                    json rooms_backup = serv->get_rooms();
                    json new_room = target_room;
                    (target_room_reservations.erase(reserve_index));
                    new_room["users"] = target_room_reservations;
                    rooms_backup = update_json(room_id, "number", "rooms", rooms_backup, new_room);
                    serv->set_rooms(rooms_backup);
                    response["code"] = 413;
                    response["message"] = "Exitted earlier than predetermined checkout succesfully";
                }
                serv->set_rooms(rooms_backup);
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
json user_reservations_response(json request, Server *serv)
{
    json response;
    string request_username = request.at("payload").at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    json rooms_backup = serv->get_rooms();
    json reservations = json::array();
    if (target_user != nullptr)
    {
        if (target_user["logged_in"] == true)
        {

            for (auto v : rooms_backup.at("rooms"))
            {
                for (auto y : v.at("users"))
                {
                    if (y.at("id") == target_user.at("id"))
                    {
                        json y_copy = y;
                        y_copy["room_number"] = v.at("number");
                        reservations.push_back(y_copy);
                    }
                }
            }
            response["code"] = 200;
            response["reservations"] = reservations;
            response["message"] = "Reservation Information Retrieved succesfully";
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

json cancel_room_response(json request, Server *serv)
{
    json response;
    json payload = request.at("payload");
    string room_id = payload.at("room_number");
    string request_username = payload.at("username");
    int cancel_bed_count = payload.at("bed_count");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    json target_room = search_json(room_id, "number", "rooms", serv->get_rooms());
    if (target_room == nullptr)
    {
        response["message"] = "Room not found";
        response["code"] = 503;
        return response;
    }
    if (target_user != nullptr)
    {
        if (target_user.at("logged_in") == true)
        {
            json rooms_backup = serv->get_rooms();
            int reserve_index = search_json_array_int_key(target_user.at("id"), "id", "users", target_room);
            if (reserve_index == -1)
            {
                response["code"] = 102;
                response["message"] = "You have not reserved  this room";
            }
            else
            {
                json target_room_reservations = target_room.at("users");
                json reservation = target_room_reservations[reserve_index];
                string checkin = reservation.at("check_in_date");
                string checkout = reservation.at("checkout_date");
                bool cancelable = smaller(*(serv->cur), Date(checkout));
                cout << getTime() << "Cancellation Request" << Date(checkout).get_string() << " " << (serv->cur)->get_string() << " " << checkin << " " << cancelable << endl;
                int previous_bed_count =
                    target_room_reservations[reserve_index].at("bed_count");
                if (cancel_bed_count > previous_bed_count)
                {
                    response["code"] = 102;
                    response["message"] = "You are cancelling more than you have reserved!";
                    return response;
                }
                if (not(cancelable))
                {
                    response["code"] = 102;
                    response["message"] = "Your room is not cancellable at this time";
                    return response;
                }
                else
                {
                    json rooms_backup = serv->get_rooms();
                    if (cancel_bed_count == previous_bed_count)
                    {
                        json new_room = target_room;
                        target_room_reservations.erase(reserve_index);
                        new_room["users"] = target_room_reservations;
                        rooms_backup = update_json(room_id, "number", "rooms", rooms_backup, new_room);
                        serv->set_rooms(rooms_backup);
                        response["code"] = 110;
                        response["message"] = "Checked out earlier than predetermined checkout succesfully";
                        return response;
                    }
                    else
                    {
                        json reserve = target_room_reservations[reserve_index];
                        reserve["bed_count"] = int(reserve["bed_count"]) - cancel_bed_count;
                        json new_room = target_room;
                        target_room_reservations[reserve_index] = reserve;
                        new_room["users"] = target_room_reservations;
                        rooms_backup = update_json(room_id, "number", "rooms", rooms_backup, new_room);
                        serv->set_rooms(rooms_backup);
                        response["code"] = 110;
                        response["message"] = "Checked out earlier than predetermined checkout succesfully";
                        return response;
                    }
                }
                serv->set_rooms(rooms_backup);
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
    string room_id = payload.at("room_number");
    string price = payload.at("price");
    string max_capacity = payload.at("max_capacity");
    string request_username = payload.at("username");
    json target_user = search_json(request_username, "user", "users", serv->get_users());
    string target_room_id = payload.at("room_number");
    json target_room = search_json(target_room_id, "number", "rooms", serv->get_rooms());
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
                int price_int = atoi(price.c_str());
                new_room["capacity"] = max_capacity_int;
                new_room["max_capacity"] = max_capacity_int;
                new_room["status"] = 0;
                new_room["price"] = price_int;
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
    json response;
    try
    {
        string request_type = request.at("type");
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
            response = logout_response(request, serv);
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
        else if (request_type == "view_empty_room_information")
        {
            response = view_empty_rooms_response(request, serv);
        }
        else if (request_type == "view_room_information")
        {
            response = view_room_information_response(request, serv);
        }

        else if (request_type == "book_room")
        {

            response = book_room_response(request, serv);
        }
        else if (request_type == "admin_check")
        {
            bool is_admin = serv->is_user_admin(request.at("payload").at("username"));
            if (is_admin)
            {
                response["code"] = 101;
                response["message"] = "Admin check passed";
            }
            else
            {
                response["code"] = 403;
                response["message"] = "Admin check failed , you don't have access to the resource";
            }
        }
        else if (request_type == "edit_information")
        {

            response = edit_information_response(request, serv);
        }
        else if (request_type == "remove_room")
        {
            response = remove_room_response(request, serv);
        }
        else if (request_type == "leave_room")
        {
            response = leave_room_response(request, serv);
        }
        else if (request_type == "modify_room")
        {
            response = modify_room_response(request, serv);
        }
        else if (request_type == "add_room")
        {

            response = add_room_response(request, serv);
        }
        else if (request_type == "user_reservations")
        {
            response = user_reservations_response(request, serv);
        }
        else if (request_type == "cancel_reserve")
        {
            response = cancel_room_response(request, serv);
        }
        serv->update_reserves_with_time();
    }
    catch (...)
    {
        response["code"] = 503;
        response["message"] = "Encountered Server error";
        auto expPtr = std::current_exception();

        try
        {
            if (expPtr)
                std::rethrow_exception(expPtr);
        }
        catch (const std::exception &e) // it would not work if you pass by value
        {
            std::cout << getTime() << e.what();
        }
    }
    return response;
}
#endif // !1
