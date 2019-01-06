#ifndef IMSI_HPP
#define IMSI_HPP
#include <stdint.h>
#include <string.h>

struct IMSI {
    uint8_t val[15];
    bool operator < (const IMSI& _imsi) const {
        return memcmp(&val[0], &_imsi.val[0], sizeof(val)) < 0;
    }
    IMSI operator = (const IMSI& _imsi) {
        if(this != &_imsi)
            memcpy(&val[0], &_imsi.val[0], sizeof(val));
        return *this;
    }
	IMSI() {
		memset(val, 0, sizeof(val));
	}
	IMSI(const char* _val) {
		int len = 15;
		if(strlen(_val) < len)
			len = strlen(_val);
		memcpy(val, _val, len);
	}
};


#endif
