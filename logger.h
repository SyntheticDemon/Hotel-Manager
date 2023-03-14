#ifndef LOGGER
#define LOGGER
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
string getTime();

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
