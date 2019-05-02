#ifndef REDISHELPER_HPP
#define REDISHELPER_HPP
#include <stdlib.h>
#include <string>
#include <sstream>
#include <hiredis/hiredis.h>
#include <iostream>
using std::string;
using std::cout;
using std::endl;
using std::stringstream;
class RedisHelper {
    public:
        RedisHelper(){}
        ~RedisHelper() {
            if(context)
                disConnect();
        }
        void connect(const string& ip, int port) {
            context = redisConnect(ip.c_str(), port);
            if(context && context->err) {
                cout<<"connect redis error"<<endl;
                exit(EXIT_FAILURE);
            }
            cout<<"connect redis success"<<endl;
        }
        void disConnect() {
            redisFree(context);
            cout<<"redis disconnect"<<endl;
            context = NULL;
        }
        void set(const string& domin, const string& key, const string& value) {
            stringstream ss;
            string command;
            ss<<"SET "<<domin<<"*"<<key<<" "<<value;
            getline(ss, command);
            redisCommand(context, command.c_str());
        }
        int get(const string& domin, const string& key, string& result) {
            stringstream ss;
            string command;
            ss<<"GET "<<domin<<"*"<<key;
            getline(ss, command);
            redisReply* reply = (redisReply*)redisCommand(context, command.c_str());
            if(reply && reply->type == REDIS_REPLY_STRING) {
                result = reply->str;
                return result.length();
            }
            else
                return 0;
        }
    private:
        redisContext* context;
};

#endif
