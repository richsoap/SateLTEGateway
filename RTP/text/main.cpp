#include "textblank.hpp"
#include "textclip.hpp"
#include "common.h"
#include <list>
#include <iostream>

using namespace std;
int main() {
    char* str0 = "Hello Hello";
    char* str1 = "World World";
    char* str2 = "! !";
    char* str3 = "\n \n";
    char buffer[128];
    int result;
    TextBlank blank0((uint8_t*)str0, 6);
    TextBlank blank1((uint8_t*)str1, 6);
    TextBlank blank2((uint8_t*)str2, 2);
    TextBlank blank3((uint8_t*)str3, 2);
    list<TextClip> data;
    data.push_back(TextClip(blank0,0,6));
    data.push_back(TextClip(blank1,0,6));
    data.push_back(TextClip(blank2,0,2));
    data.push_back(TextClip(blank3,3,2));
    result = TextClip::stringFlush(data, (uint8_t*)buffer, 128);
    if(result > 0)
        cout<<buffer<<endl;
    cout<<result<<endl;
    data.pop_back();
    data.push_back(TextClip(blank3,0,300));
    result = TextClip::stringFlush(data, (uint8_t*)buffer, 128);
    if(result > 0)
        cout<<buffer<<endl;
    cout<<result<<endl;
    return 0;
}
