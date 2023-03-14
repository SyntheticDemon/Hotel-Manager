#ifndef UTILS
#define UTILS UTILS.CPP
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
        if (month >= 12 - int(days / 30) && this->day > 30 - days)
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

bool smaller(Date first_start, Date first_end)
{
    int first_start_epoch = first_start.year * 365 + first_start.month * 30 + first_start.day;
    int first_end_epoch = first_end.year * 365 + first_end.month * 30 + first_end.day;
    return (first_end_epoch < first_start_epoch);
}

bool have_overlap(Date first_start, Date first_end, Date second_start, Date second_end)
{
    int first_start_epoch = first_start.year * 365 + first_start.month * 30 + first_start.day;
    int first_end_epoch = first_end.year * 365 + first_end.month * 30 + first_end.day;
    int second_start_epoch = second_start.year * 365 + second_start.month * 30 + second_start.day;
    int second_end_epoch = second_end.year * 365 + second_end.month * 30 + second_end.day;
    return not(
        (
            (first_end_epoch > second_end_epoch) && (first_start_epoch > second_start_epoch)) ||
        ((second_end_epoch > first_end_epoch) && (second_start_epoch > first_start_epoch)));
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
#endif // !1
