#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include <iostream>
using namespace std;
#include <fstream>

json read_json_f(string &location)
{
    std::ifstream config_stream(location);
    json config = json::parse(config_stream);
    return config;
}

vector<string> tokenize(string line)
{
    vector<string> tokens;
    stringstream check1(line);

    string intermediate;
    while (getline(check1, intermediate, ' '))
    {
        tokens.push_back(intermediate);
    }
    return tokens;
}