#ifndef DATE
#define DATE
#include <string>
using namespace std;

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

bool smaller(Date first_start, Date first_end);

bool have_overlap(Date first_start, Date first_end, Date second_start, Date second_end);

#endif // !1