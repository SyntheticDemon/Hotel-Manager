
#include "json_utils.h"
#include "my_utils.h"
#include "date.h"
#include "logger.h"
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
#include "server.h"
#include <semaphore>
#include "responses.h"
#include <thread>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
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
                if (smaller((*this->cur), checkout))
                {
                    cout << getTime() << "Date was smaller therefore stays" << endl;
                    new_reservations.push_back(v_2);
                }
            }
            v["users"] = new_reservations;
        }
        v["capacity"] = this->get_room_capacity(v);
    }
    json new_rooms = rooms_backup;
    this->set_rooms(new_rooms);
}
int Server::get_room_capacity(json target_room)
{

    int capacity = target_room.at("max_capacity");
    for (auto v : target_room.at("users"))
    {
        string check_in_date = v.at("check_in_date");
        string checkout_date = v.at("checkout_date");
        if (not(smaller((*this->cur), Date(check_in_date))) && smaller(*(this->cur), Date(checkout_date)))
        {
            capacity -= int(v.at("bed_count"));
        }
    }
    return capacity;
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
        smaller(end_date, start_date) || smaller(end, (*this->cur)) || smaller(start, (*this->cur));
    cout << getTime() << "Checking Period Start : " << start << " End : " << end << " Result " << result << " Server Time " << (*this->cur).get_string() << endl;
    cout << getTime() << "First " << smaller(start_date, end_date) << " Second "
         << smaller(end, (*this->cur)) << " Third " << smaller(start, (*this->cur)) << endl;
    return result;
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
    while (true)
    {
        update_reserves_with_time();
        this->Server::server_accept(sizeof(address), address);
    }
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
    this->log_file = log_file;
    // Logger l;
    // this->logger = &l;

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
    string log_file;
    // setup server logging
    if (argc != 2)
    {
        log_file = "default_log.txt";
    }
    else
    {
        log_file = string(argv[1]);
    }
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