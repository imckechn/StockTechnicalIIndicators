#include <string>
enum Exchanges { none, all, nyse, nasdaq };

struct basicData {
     std::string date;
    double high, low, close, adjustedClose;
    int volume;
};

struct data {
    basicData *row;
    int length;
};

