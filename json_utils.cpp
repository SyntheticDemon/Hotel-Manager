

#include <iostream>
#include <map>
#include <vector>
#include <string.h>
#include <mutex>
#include <arpa/inet.h>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include "server.h"
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "my_utils.h"
#include "logger.h"
#include "responses.h"
json read_json_f(string &location)
{
    std::ifstream config_stream(location);
    json config = json::parse(config_stream);
    return config;
}

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

int search_json_array_int_key(int id, string id_field, string sub_path, json input)
{
    int x = 0;
    for (auto v : input.at(sub_path))
    {
        cout << v << " " << id;
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