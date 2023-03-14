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

vector<string> tokenize(string line, char delimiter)
{
    vector<string> tokens;
    stringstream check1(line);

    string intermediate;
    while (getline(check1, intermediate, delimiter))
    {
        tokens.push_back(intermediate);
    }
    return tokens;
}

class Date
{
public:
    int year;
    int day;
    int month;
    string get_string()
    {
        return (to_string(year) + "-" + to_string(month) + "-" + to_string(day));
    }
    void move(int days);
    Date(string date);
};

void Date::move(int days)
{
    if (this->day > 30 - days)
    {
        if (month >= 12 - int(days / 30)  && this->day > 30 - days)
        {
            this->year += 1;
        }
        this->day += days;
        this->month += 1;
        this->day = this->day % 30;
        this->month = this->month % 12;
    }
    else
    {
        this->day += days;
    }
}

Date::Date(string date)
{
    vector<string> tokens = tokenize(date, '-');
    int day = stoi(tokens[2]);
    int month = stoi(tokens[1]);
    int year = stoi(tokens[0]);
    this->day = day;
    this->month = month;
    this->year = year;
}
