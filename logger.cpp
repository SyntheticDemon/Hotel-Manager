#ifndef LOGGER
#define LOGGER LOGGER.CPP
#include <ostream>
#include <string>
#include <cassert>
#include <ctime>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <ctime>
using namespace std;
string getTime()
{
    // freopen(output_file.c_str(), "a", stdout);
    auto sys_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    string time = string(ctime(&sys_time));
    std::replace(time.begin(), time.end(), '\n', ' ');
    return "[" + time + "] ";
}
class Logger
{
public:
    Logger() = default;
    Logger &operator<<(string input)
    {
        cout << getTime() << input;
        return (*this);
    }

};

#endif // !1
