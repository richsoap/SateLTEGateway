#ifndef TEXTBLANK_H
#define TEXTBLANK_H

#include "common.h"

class TextBlank {
    public:
        uint8_t* buffer;
        int len;
        TextBlank(uint8_t* _buffer = NULL, int _len = 0): buffer(_buffer),len(_len) {}
        void destory() {
            if(buffer != NULL && len != 0)
                delete buffer;
        }

};

#endif
