#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#define BUFFER_SIZE 10000
#include "utils.cpp"
using namespace std;
void showMenu()
{
    cout << "<< 1. View user information" << endl;
    cout << "<< 2. View all users" << endl;
    cout << "<< 3. View rooms information" << endl;
    cout << "<< 4. Booking" << endl;
    cout << "<< 5. Canceling" << endl;
    cout << "<< 6. Pass day" << endl;
    cout << "<< 7. Edit information" << endl;
    cout << "<< 8. Leaving room" << endl;
    cout << "<< 9. Rooms" << endl;
    cout << "<< 0. Logout" << endl;
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
    string message = jobject.dump();
    const char *m = message.c_str();
    return send(this->client_fd, m, strlen(m), 0);
}

json Connector::connector_receive()
{
    char temp_buffer[BUFFER_SIZE] = {0};

    if (read(this->client_fd, temp_buffer, BUFFER_SIZE))
    {
        std::string cpp_message(temp_buffer);
        return json::parse(cpp_message);
    }
    else
    {

        cout << ("Could not read from server FD") << endl;
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
    cout << "Server Ip " << server_ip << " " << server_port << endl;
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
    json logout(string username);
    json signup(vector<string> tokens);
    json login(vector<string> tokens);
    json all_users(vector<string> tokens);
    json view_user_information(vector<string> tokens);
    json pass_day(vector<string> tokens);
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
    cout << "Sending Request" << logout_form << endl;
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    cout << "Server Response " << response.dump(4) << endl;
    return response;
}

json Client::all_users(vector<string> tokens)
{
    json response;
    json all_users_form;
    all_users_form["username"] = this->active_username;
    json request = create_request(all_users_form, "all_users");
    cout << "Sending Request" << all_users_form << endl;
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    cout << "Server Response " << response.dump(4) << endl;
    return response;
}

json Client::login(vector<string> tokens)
{
    j