#ifndef STOPWATCH_H
#define STOPWATCH_H
#include<sys/time.h>
#include<stdint.h>
#include<list>
#include<vector>
#include<string>
using std::list;
using std::vector;
using std::string;
class Stopwatch {
    private:
    vector<list<uint64_t>> records;
    vector<uint64_t> start;
    vector<uint64_t> count;
    uint64_t trigger;
    uint64_t number;
    string filename;
    timeval timestamp;
    void flush(list<uint64_t>& buffer, int code);
    public:
    Stopwatch(uint64_t _number = 3, uint64_t _trigger = 1000, string _filename = "Stopwatch");
    ~Stopwatch();
    void reset(uint64_t code);
    void resetAll();
    void record(uint64_t code);
};

#endif
