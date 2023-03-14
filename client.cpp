#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "logger.cpp"
#include <iostream>
#define BUFFER_SIZE 10000
#include "utils.cpp"
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
    json logout(string username);
    json signup(vector<string> tokens);
    json login(vector<string> tokens);
    json all_users(vector<string> tokens);
    json view_user_information(vector<string> tokens);
    json book_room(vector<string> tokens);
    json pass_day(vector<string> tokens);
    json view_room_information(vector<string> tokens);
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

json Client::login(vector<string> tokens)
{
    json response;
    json login_form;
    login_form["username"] = tokens[1];
    login_form["password"] = tokens[2];
    json request = create_request(login_form, "login");
    cout << "Sending Request" << login_form << endl;
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    cout << "Server Response " << response.dump(4) << endl;
    return response;
}

json Client::view_user_information(vector<string> tokens)
{
    json response;
    json user_formation_form;
    user_formation_form["username"] = this->active_username;
    json request = create_request(user_formation_form, "view_user_information");
    cout << "Sending Request" << user_formation_form << endl;
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    cout << "Server Response " << response.dump(4) << endl;
    return response;
}

json Client::pass_day(vector<string> tokens)
{
    json response;
    json pass_day_form;
    int days = atoi(tokens[1].c_str());
    pass_day_form["days"] = days;
    pass_day_form["username"] = this->active_username;
    json request = create_request(pass_day_form, "pass_day");
    cout << "Sending Request" << pass_day_form << endl;
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    cout << "Server Response " << response.dump(4) << endl;
    return response;
}

json Client::signup(vector<string> tokens)
{
    json response;
    json signup_form;
    json sign_up_check_form;
    sign_up_check_form["username"] = tokens[1];
    json request = create_request(sign_up_check_form, "signup_check");
    cout << "Sending Request" << sign_up_check_form << endl;
    this->conn->connector_send(request);
    response = this->conn->connector_receive();
    cout << "Server Response " << response.dump(4) << endl;
    if (response["code"] == 313)
    {
        cout << "Username check passed Enter the rest of the data " << endl;
        string password, purse, phone, address;
        cout << "<< password : ";
        getline(std::cin, password);
        cout << "<< purse : ";
        getline(std::cin, purse);
        cout << "<< phone : ";
        getline(std::cin, phone);
        cout << "<< address : ";
        getline(std::cin, address);
        signup_form["password"] = password;
        signup_form["purse"] = purse;
        signup_form["phone"] = phone;
        signup_form["address"] = address;
        signup_form["username"] = tokens[1];
        cout << "Sending Request" << signup_form << endl;
        json signup_request = create_request(signup_form, "signup");
        this->conn->connector_send(signup_request);
        response = this->conn->connector_receive();
        cout << "Server Response " << response.dump(4) << endl;
    }
    else
    {
    }

    return response;
}

vector<string> Client::receive_and_tokenize_input()
{
    string input_string;
    cout << "<< ";
    getline(std::cin, input_string);
    vector<string> tokens = tokenize(input_string, ' ');
    // inputs.push_back(tokenize(input_string));
    return tokens;
}

Client::Client()
{
    Connector conn = Connector("./jsons/config.json");
    this->active_username = "nil";
    this->conn = &conn;
}

void Client::run()
{

    while (true)
    {
        json response;
        vector<string> tokens = receive_and_tokenize_input();
        string command = tokens[0];
        if (command == "login")
        {
            response = this->login(tokens);
            if (response.at("code") == 230)
            {
                cout << "You are now logged in" << endl;
                this->active_username = response.at("username");
                showMenu();
            }
        }
        else if (command == "signup")
        {

            response = this->signup(tokens);
        }
        else if (command == "logout")
        {
            response = this->logout(this->active_username);
            if (response["code"] == 230)
            {
                this->active_username = "nil";
            }
        }
        else if (command == "pass_day")
        {
            response = this->pass_day(tokens);
        }
        else if (command == "all_users")
        {

            response = this->all_users(tokens);
        }
        else if (command == "view_user_information")
        {
            response = this->view_user_information(tokens);
        }

        // cout << "Server Response " << response << endl;

        cout << endl;
    }
}
int main(int argc, char const *argv[])
{
    Client client = Client();
    client.run();
    // sleep(2);
    // send(sock, hello, strlen(hello), 0);
    // sleep(2);
    // send(sock, hello, strlen(hello), 0);

    // close(client_fd);
    return 0;
}