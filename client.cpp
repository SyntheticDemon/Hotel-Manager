

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/tee.hpp>
#include <filesystem>
#include <bits/stdc++.h>
#include <sys/socket.h>
#include <unistd.h>
#include "logger.h"
#include <iostream>
#define BUFFER_SIZE 10000
#include "my_utils.h"
#include "json_utils.h"

#include <exception>
using namespace std;
void showMenu()
{
    cout << getTime() << "<< 1. View user information" << endl;
    cout << getTime() << "<< 2. View all users" << endl;
    cout << getTime() << "<< 3. View rooms information" << endl;
    cout << getTime() << "<< 4. Booking" << endl;
    cout << getTime() << "<< 5. Canceling" << endl;
    cout << getTime() << "<< 6. Pass day" << endl;
    cout << getTime() << "<< 7. Edit information" << endl;
    cout << getTime() << "<< 8. Leaving room" << endl;
    cout << getTime() << "<< 9. Rooms" << endl;
    cout << getTime() << "<< 0. Logout" << endl;
    return;
}
class Connector
{
public:
    string server_ip;
    int server_port;
    int client_fd;
    int connector_shutdown();
    int connector_send(json jobject);
    int connector_disconnect();
    json connector_receive();
    Connector(string config_location);

private:
    string serialize(json request);
    json deserialize(string resposne);
};
int Connector::connector_send(json jobject)
{
    string message = jobject.dump(4);
    cout << getTime() << "Sending Request" << message << endl;
    const char *m = message.c_str();
    return send(this->client_fd, m, strlen(m), 0);
}

json Connector::connector_receive()
{
    char temp_buffer[BUFFER_SIZE] = {0};

    if (read(this->client_fd, temp_buffer, BUFFER_SIZE))
    {
        std::string cpp_message(temp_buffer);
        json response = json::parse(cpp_message);
        cout
            << getTime() << "Server Response " << response.dump(4) << endl;
        return response;
    }
    else
    {

        cout << getTime() << ("Could not read from server FD") << endl;
        return nullptr;
    }
}

int Connector::connector_disconnect()
{
    return close(this->client_fd);
}

Connector::Connector(string config_location)
{
    int sock = 0;
    json config = read_json_f(config_location);
    string server_ip = config.at("hostName");
    int server_port = config.at("commandChannelPort");
    cout << getTime() << "Server Ip " << server_ip << " " << server_port << endl;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\nInvalid address/ Address not supported \n");
        exit(EXIT_FAILURE);
    }

    if ((connect(sock, (struct sockaddr *)&serv_addr,
                 sizeof(serv_addr))) < 0)
    {
        printf("\nConnection Failed \n");
        exit(EXIT_FAILURE);
    }
    this->server_ip = server_ip;
    this->server_port = server_port;
    this->client_fd = sock;
}

class Client
{
private:
    Connector *conn;
    vector<vector<string>> inputs;

public:
    void run();
    Client();
    string active_username;
    vector<string> receive_and_tokenize_input();
    void create_log_directory(string username);

    json logout(string username);
    json signup(vector<string> tokens);
    json login(vector<string> tokens);
    json all_users(vector<string> tokens);
    json view_user_information(vector<string> tokens);
    json book_room(vector<string> tokens);
    json edit_information(vector<string> tokens);
    json pass_day(vector<string> tokens);
    json view_room_information(vector<string> tokens);
    json rooms_response(vector<string> tokens);
    json leave_room(vector<string> tokens);
    json cancel_room(vector<string> tokens);
    string get_active_log_file() { return this->active_log_file; }

private:
    string active_log_file;
};

json create_request(json payload, json type)
{
    json request;
    request["payload"] = payload;
    request["type"] = type;
    return request;
}

json Client::logout(string username)
{

    json response;
    json logout_form;
    logout_form["username"] = username;
    json request = create_request(logout_form, "logout");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}

json Client::all_users(vector<string> tokens)
{
    json response;
    json all_users_form;
    all_users_form["username"] = this->active_username;
    json request = create_request(all_users_form, "all_users");

    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}

json Client::rooms_response(vector<string> tokens)
{
    json first_request_form;
    first_request_form["username"] = this->active_username;
    json first_request = create_request(first_request_form, "admin_check");
    json first_response =
        this->conn->connector_send(first_request);
    first_response = this->conn->connector_receive();
    if (first_response.at("code") == 403)
    {
        return first_response;
    }
    else
    {
        cout << getTime() << "Enter the rest of your command" << endl;
        vector<string> secondery_tokens = receive_and_tokenize_input();
        json main_request;
        string type;
        main_request["username"] = this->active_username;
        main_request["room_number"] = secondery_tokens[1];
        if (secondery_tokens[0] == "add")
        {
            main_request["max_capacity"] = secondery_tokens[2];
            main_request["price"] = secondery_tokens[3];
            type = "add_room";
        }
        else if (secondery_tokens[0] == "modify")
        {
            main_request["new_max_capacity"] = secondery_tokens[2];
            main_request["new_price"] = secondery_tokens[3];
            type = "modify_room";
        }
        else if (secondery_tokens[0] == "remove")
        {
            type = "remove_room";
        }
        else
        {
            json response;
            response["code"] = 503;
            response["message"] = "invalid input";
            cout << getTime() << response.dump(4);
            return response;
        }
        json final_request = create_request(main_request, type);
        this->conn->connector_send(final_request);
        json response = this->conn->connector_receive();
        return response;
    }
}
json Client::cancel_room(vector<string> tokens)
{
    json first_request_payload;
    json second_response;
    string first_type = "user_reservations";
    first_request_payload["username"] = this->active_username;
    json first_request = create_request(first_request_payload, first_type);
    this->conn->connector_send(first_request);
    json first_response = this->conn->connector_receive();
    sleep(0.5);
    cout << getTime() << "Pick a room to cancel out of the resulting rooms" << endl;
    vector<string> secondery_tokens = receive_and_tokenize_input();
    if (first_response["code"] == 200)
    {
        if (secondery_tokens.size() == 3)
        {
            json second_request_payload;
            second_request_payload["username"] = this->active_username;
            second_request_payload["room_number"] = secondery_tokens[1];
            second_request_payload["bed_count"] = atoi(secondery_tokens[2].c_str());
            json second_request = create_request(second_request_payload, "cancel_reserve");
            this->conn->connector_send(second_request);
            second_response = this->conn->connector_receive();
            return second_response;
        }
        else
        {
            throw(503);
        }
    }
    return second_response;
}
json Client::leave_room(vector<string> tokens)
{
    sleep(0.5);
    cout << getTime() << "Enter the rest of your command " << endl;
    vector<string> secondery_tokens = receive_and_tokenize_input();

    json main_request;
    string type;
    main_request["username"] = this->active_username;
    main_request["room_number"] = secondery_tokens[1];
    if (secondery_tokens[0] == "room")
    {
        type = "leave_room";
    }
    else
    {
        json response;
        response["code"] = 503;
        response["message"] = "invalid input";
        cout << getTime() << response.dump(4);
        return response;
    }
    json final_request = create_request(main_request, type);
    this->conn->connector_send(final_request);
    json response = this->conn->connector_receive();
    return response;
}
json Client::login(vector<string> tokens)
{
    json response;
    json login_form;
    login_form["username"] = tokens[1];
    login_form["password"] = tokens[2];
    json request = create_request(login_form, "login");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}

json Client::book_room(vector<string> tokens)
{
    cout << getTime() << "Enter Your Booking Request" << endl;
    vector<string> secondery_tokens = receive_and_tokenize_input();
    if (secondery_tokens.size() != 5)
    {
        throw(503);
    }
    {
        json response;
        string room_number = secondery_tokens[1];
        string bed_count = secondery_tokens[2];
        string check_in_date = secondery_tokens[3];
        string checkout_date = secondery_tokens[4];
        json booking_form;
        booking_form["username"] = this->active_username;
        booking_form["room_number"] = room_number;
        booking_form["bed_count"] = bed_count;
        booking_form["check_in_date"] = check_in_date;
        booking_form["checkout_date"] = checkout_date;
        json request = create_request(booking_form, "book_room");
        this->conn->connector_send(request);
        response = this->conn->connector_receive();
        return response;
    }
}

json Client::view_user_information(vector<string> tokens)
{
    json response;
    json user_formation_form;
    user_formation_form["username"] = this->active_username;
    json request = create_request(user_formation_form, "view_user_information");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}
json Client::edit_information(vector<string> tokens)
{
    json response;
    cout << getTime() << "Enter new information" << endl;
    string password, phone, address;
    cout << getTime() << "<< New password : ";
    getline(std::cin, password);
    cout << getTime() << "<< Phone : ";
    getline(std::cin, phone);
    cout << getTime() << "<< Address : ";
    getline(std::cin, address);
    json edit_information_form;
    edit_information_form["username"] = this->active_username;
    edit_information_form["new_password"] = password;
    edit_information_form["address"] = address;
    edit_information_form["phone"] = phone;
    json request = create_request(edit_information_form, "edit_information");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}

// json Client::cancel_reserve(){
// vector<string> new_tokens = this->receive_and_tokenize_input();
// }
json Client::view_room_information(vector<string> tokens)
{
    json response;
    json user_formation_form;
    user_formation_form["username"] = this->active_username;
    json request = create_request(user_formation_form, "view_room_information");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}

json Client::pass_day(vector<string> tokens)
{
    json response;
    json pass_day_form;
    string days_input;
    cout << getTime() << "Type the number of days to fast forward " << endl;
    getline(std::cin, days_input);
    int days;
    days = atoi(days_input.c_str());
    pass_day_form["days"] = days;
    pass_day_form["username"] = this->active_username;
    json request = create_request(pass_day_form, "pass_day");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    return response;
}

json Client::signup(vector<string> tokens)
{
    json response;
    json signup_form;
    json sign_up_check_form;
    sign_up_check_form["username"] = tokens[1];
    json request = create_request(sign_up_check_form, "signup_check");
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    if (response["code"] == 313)
    {
        cout << getTime() << "Username check passed Enter the rest of the data " << endl;
        string password, purse, phone, address;
        cout << getTime() << "<< password : ";
        getline(std::cin, password);
        cout << getTime() << "<< purse : ";
        getline(std::cin, purse);
        cout << getTime() << "<< phone : ";
        getline(std::cin, phone);
        cout << getTime() << "<< address : ";
        getline(std::cin, address);
        signup_form["password"] = password;
        signup_form["purse"] = purse;
        signup_form["phone"] = phone;
        signup_form["address"] = address;
        signup_form["username"] = tokens[1];
        json signup_request = create_request(signup_form, "signup");
        this->conn->connector_send(signup_request);
        response = this->conn->connector_receive();
    }

    return response;
}

vector<string> Client::receive_and_tokenize_input()
{
    string input_string;
    cout << getTime() << "<< ";
    getline(std::cin, input_string);
    vector<string> tokens = tokenizeee(input_string, ' ');
    // inputs.push_back(tokenize(input_string));
    return tokens;
}

Client::Client()
{
    Connector conn = Connector("./jsons/config.json");
    this->active_username = "nil";
    this->conn = &conn;
}
void Client::create_log_directory(string username)
{
    string file_command = "mkdir -p";
    file_command += " logs/";
    string username_file_director = this->active_username;
    file_command += username_file_director;
    system(file_command.c_str());
}

void Client::run()
{

    while (true)
    {
        if (this->active_username != "nil")
        {
            this->create_log_directory(this->active_username);
            showMenu();
        }
        else
        {
            cout << getTime() << "Please Login or Signup if you don't have an account" << endl;
            sleep(0.25);
        }
        json response;
        try
        {
            vector<string> tokens = receive_and_tokenize_input();
            string command = tokens[0];
            if (command == "login" && tokens.size() == 3)
            {
                response = this->login(tokens);
                if (response.at("code") == 230)
                {
                    cout << getTime() << "You are now logged in" << endl;
                    this->active_username = response.at("username");
                }
                else if (response.at("code") == 430)
                {
                    this->active_username = "nil";
                }
            }
            else if (command == "signup")
            {

                response = this->signup(tokens);
            }
            else if (command == "0")
            {
                response = this->logout(this->active_username);
                if (response["code"] == 230)
                {
                    this->active_username = "nil";
                }
            }
            else if (command == "6")
            {
                response = this->pass_day(tokens);
            }
            else if (command == "2")
            {

                response = this->all_users(tokens);
            }
            else if (command == "1")
            {
                response = this->view_user_information(tokens);
            }
            else if (command == "3")
            {
                response = this->view_room_information(tokens);
            }
            else if (command == "4")
            {
                response = this->book_room(tokens);
            }
            else if (command == "5")
            {
                response = this->cancel_room(tokens);
            }
            else if (command == "7")
            {
                response = this->edit_information(tokens);
            }
            else if (command == "8")
            {
                response = this->leave_room(tokens);
            }
            else if (command == "9")
            {
                response = this->rooms_response(tokens);
            }
            else
            {
                throw(503);
                // cout << getTime() << "Server Response " << response << endl;
            }
        }
        catch (int error_code)
        {
            response["code"] = 503;
            response["message"] = "User command parsing failed (local error)";
            cout << getTime() << "Local Response" << response.dump(4) << endl;
        }
        catch (exception)
        {
            response["code"] = 503;
            response["message"] = "User command parsing failed (local error)";
            cout << getTime() << "Local Response" << response.dump(4) << endl;
        }
    }
}
int main(int argc, char const *argv[])
{

    Client client = Client();
    // typedef boost::iostreams::tee_device<std::ostream, std::ofstream> Tee;
    // typedef boost::iostreams::stream<Tee> TeeStream;
    // std::ofstream file("foo.out");
    // Tee tee(std::cout, file);
    // TeeStream both(tee);

    client.run();

    // both << "this goes to both std::cout and foo.out" << std::endl;
    // sleep(2);
    // send(sock, hello, strlen(hello), 0);
    // sleep(2);
    // send(sock, hello, strlen(hello), 0);

    // close(client_fd);
    return 0;
}