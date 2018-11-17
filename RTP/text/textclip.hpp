#ifndef TEXTLIST_H
#define TEXTLIST_H
#include "common.h"
#include <list>
#include "textblank.hpp"
class TextClip {
    private:
        TextBlank data;
        int offset;
        int len;
    public:
        TextClip(TextBlank _data, int _offset = 0, int _len = 0):data(_data), 
                                                                offset(_offset), 
                                                                len(_len) {};
        TextClip& setOffset(int _offset);
        TextClip& setLen(int _len);
        static int flush(std::list<TextClip> data, uint8_t* buffer, int buffer_size);
        static int stringFlush(std::list<TextClip>data, uint8_t* buffer, int buffer_size);
};

TextClip& TextClip::setOffset(int _offset) {
    offset = _offset;
    return *this;
}
TextClip& TextClip::setLen(int _len) {
    len = _len;
    return *this;
}

/*
 *
 * return:
 * +: success
 * -1: void clip
 * -2: not enough mem
 *
 */
int TextClip::flush(std::list<TextClip> data, uint8_t* buffer, int buffer_size) {
    int offset = 0;
    int count = 0;
    for(std::list<TextClip>::iterator it = data.begin(); it != data.end(); it ++) {
        count = it->data.len-it->offset<it->len?it->data.len-it->offset:it->len;
        std::cout<<count<<" :  ";
        if(count < 0)
            return -1;
        else if(count + offset > buffer_size)
            return -2;
        memcpy(buffer+offset, it->data.buffer + it->offset, count);
        offset += count;
    }
    return offset;
}

int TextClip::stringFlush(std::list<TextClip> data, uint8_t* buffer, int buffer_size) {
    int result = flush(data, buffer, buffer_size);
    if(result < 0)
        return result;
    if(result == buffer_size)
        return -2;
    buffer[result ++] = '\0';
    return result;
}

#endif
