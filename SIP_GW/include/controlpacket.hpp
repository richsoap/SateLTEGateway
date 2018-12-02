#ifndef CONTROLPACKET
#define CONTROLPACKET
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define CONTROL_UNKNOWN
#define CONTROL_ADD 0x01
#define CONTROL_REMOVE 0x02

#define CONTROL_AMR 0x01
#define CONTROL_GSM 0x02
using namespace std;
struct ControlPacket {
  	uint8_t command;
  	uint8_t payload;
  	uint8_t code;
  	sockaddr_in addr;
  	string callid;
    void init(uint8_t _command, uint8_t _pt, uint8_t _code, const sockaddr& _addr, const string& _callid) {
      command = _command;
      payload = _pt;
      code = _code;
      memcpy(&addr, &_addr, sizeof(sockaddr));
      callid = string(_callid);
    }
    int toBuffer(char* buffer) {
      int result = 0;
      buffer[result ++] = command;
      buffer[result ++] = payload;
      buffer[result ++] = code;
      memcpy[buffer + result, &addr, sizeof(sockaddr)];
      result += sizeof(sockaddr);
      memcpy[buffer + result, callid.c_str(), callid.length()];
      result += callid.length();
      return result;
    }
    void fromBuffer(char* buffer, int length) {
      int offset = 3+sizeof(sockaddr);
      memcpy(this, buffer, 3+sizeof(sockaddr));
      callid = string(buffer + offset, length - offset);
    }
};
#endif
