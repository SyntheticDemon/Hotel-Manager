#ifndef RESPOSNES
#define RESPONSES
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class Server;
#include "server.h"
json pass_day_response(json request, Server *serv);
json logout_response(json request, Server *serv);
json login_response(json request, Server *serv);
json view_user_information_response(json request, Server *serv);
void remove_password_from_users(json &users);
json all_users_response(json request, Server *serv);
json view_room_information_response(json request, Server *serv);
json edit_information_response(json request, Server *serv);
json book_room_response(json request, Server *serv);
json signup_check_response(json request, Server *serv);
json signup_response(json request, Server *serv);
json modify_room_response(json request, Server *serv);
json remove_room_response(json request, Server *serv);
json add_room_response(json request, Server *serv);
json generate_response_fx(json request, Server *serv);
json cancel_room_response(json request, Server *serv);
json user_reservations_response(json request, Server *serv);
json leave_room_response(json request, Server *serv);

#endif