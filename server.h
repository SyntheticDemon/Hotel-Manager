
#ifndef SERVER
#define SERVER
#include "my_utils.h"
#include "json_utils.h"
#include "responses.h"
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
// #include <semaphore>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
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
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Server
{
public:
    int port;
    string log_file;
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
    
    ofstream out_file;
    
    Server(string config_location, string users_location, string rooms_location, string log_file);

    string serialize(json request);
    json deserialize(string resposne);

    bool is_user_admin(string username); // TODO Complete ,
    map<int, char *> client_buffers;
    bool authenticate(json request);
    int overlap_occuptations(string start, string end, json target_room);
    bool invalid_date(string start, string end);
    void update_reserves_with_time();
    int get_room_capacity(json target_room);
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
        // pthread_mutex_lock(&this->room_data_lock);
        this->rooms = new_rooms;
        // pthread_mutex_unlock(&this->room_data_lock);
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
        // pthread_mutex_lock(&this->user_data_lock);
        this->users = new_users;
        // pthread_mutex_unlock(&this->user_data_lock);
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
#endif