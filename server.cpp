// #include <QtCore>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <string.h>
#include <mutex>
#include <arpa/inet.h>
#include <fstream>
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

class Server
{
public:
    struct sockaddr_in address;
    int port;
    json users;
    json config;
    json rooms;
    int server_fd;
    vector<int> client_fds;
    map<int, char *> client_buffers;
    int server_shutdown();
    int server_send(int client_fd, json jobject);
    int server_accept(int addrlen, struct sockaddr_in address);
    int server_disconnect(int client_fd);
    json server_receive(int client_fd);
    Server(string config_location, string users_location, string rooms_location);

private:
    string serialize(json request);
    json deserialize(string resposne);
};

void handle_client(Server *serv, int client_fd)
{
    cout << "Started Client Thread " << client_fd << endl;
    char new_client_buffer[BUFFER_SIZE] = {0};
    serv->client_buffers.insert(std::pair<int, char *>(client_fd, new_client_buffer));
    while (true)
    {
        cout << "Attempting to read " << endl;
        if (read(client_fd, new_client_buffer, BUFFER_SIZE) == 0)
        {
            break;
        };
        cout << "Read Complete :" << new_client_buffer << endl;
    }
    close(client_fd);
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
        cout << "Allocating Resources for this consumer (A buffer and A thread) " << new_socket << endl;
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

int Server::server_send(int client_fd, json jobject)
{

    string message = this->serialize(jobject);
    const char *m = message.c_str();
    return send(client_fd, m, strlen(m), 0);
}

json Server::deserialize(string json)
{
    return json::parse(json);
}

string Server::serialize(json jobject)
{
    return jobject.dump();
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

int Server::server_disconnect(int client_fd)
{
    return close(client_fd);
}

Server::Server(string config_location, string users_location, string rooms_location)
{
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    this->config = read_json_f(config_location);
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        // TODO Log
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
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
    this->users = read_json_f(users_location);
    int peers = this->users.count("rooms");
    if (listen(server_fd, peers) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    this->port = port;
    this->address = address;
    this->server_fd = server_fd;
    // read files

    while (true)
        this->Server::server_accept(addrlen, address);
}

int main(int argc, char const *argv[])
{

    string users_location = "jsons/users.json";
    string rooms_location = "jsons/rooms.json";
    string configs_location = "jsons/config.json";
    string initial_date;
    cout << "Enter initial date \n";
    cin >> initial_date;
    // while (true)
    // {
    //     string initial_input;
    //     cin >> initial_input;
    //     vector<string> inp = split(initial_input, ' ');
    //     if (inp[0] == "setTime")
    //     {
    //         cout << inp[0];
    //         initial_date = inp[1];
    //         break;
    //     }
    // }
    Server new_server = Server(configs_location, users_location, rooms_location);

    // while (true)
    // {
    // }

    //     QString val;
    //     QFile file;
    //     file.setFileName("config.json");
    //     file.open(QIODevice::ReadOnly | QIODevice::Text);
    //     val = file.readAll();
    //     file.close();
    //     QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
    //     QJsonObject sett2 = d.object();
    //     auto a = sett2["hostName"];
    //     // qDebug() <<;

    return 0;
}