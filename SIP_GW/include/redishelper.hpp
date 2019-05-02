#ifndef REDISHELPER_HPP
#define REDISHELPER_HPP
#include <stdlib.h>
#include <string>
#include <hiredis/hiredis.h>
#include <iostream>
using std::string;
using std::cout;
using std::endl;
using std::stringstreaem;
class RedisHelper {
    public:
        RedisHelper();
        void connect(const string& ip, int port) {
            context = ::redisConnect(ip.c_str(), port);
            if(context && context->err) {
                cout<<"connect redis error"<<endl;
                exit(EXIT_FAILURE);
            }
            cout<<"connect redis success"<<endl;
        }
        void disConnect() {
            ::redisFree(context);
            cout<<"redis disconnect"<<endl;
        }
        void set(const string& domin, const string& key, const string& value) {
            stringstream ss;
            string command;
            ss<<"SET "<<domin<<"-"<<key<<" "<<value;
            ss>>command;
            ::redisCommand(context, command.c_str());
        }
        string get(const string& domin, const string& key) {
            stringstream ss;
            string command;
            ss<<"GET "<<domin<<"-"<<key;
            ss>>command;
            redisReply* reply = (redisReply*)::redisCommand(context, command.c_str());
            if(reply && reply->type == REDIS_REPLY_STRING)
                command = reply->str;
            return command;
        }
    private:
        redisContext* context;
}

#endif
