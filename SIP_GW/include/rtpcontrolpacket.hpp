#ifndef RTPCONTROLPACKET_H
#define RTPCONTROLPACKET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define CONTROL_UNKNOWN 0x00
#define CONTROL_ADD 0x01
#define CONTROL_REMOVE 0x02

#define CONTROL_AMR 0x01
#define CONTROL_GSM 0x02
using namespace std;
struct ControlPacket {
  	uint8_t command;
  	uint8_t payload;
  	uint8_t code;
    uint8_t slot;
  	sockaddr_in addr;
  	string callid;
    void init(uint8_t _command, uint8_t _pt, uint8_t _code,uint8_t _slot ,const sockaddr& _addr, const string& _callid) {
      command = _command;
      payload = _pt;
      code = _code;
      slot = _slot;
      memcpy(&addr, &_addr, sizeof(sockaddr));
      callid = string(_callid);
    }
    int toBuffer(char* buffer) {
      int result = 0;
      buffer[result ++] = command;
      buffer[result ++] = payload;
      buffer[result ++] = code;
      buffer[result ++] = slot;
      memcpy(buffer + result, &addr, sizeof(sockaddr_in));
      result += sizeof(sockaddr_in);
      memcpy(buffer + result, callid.c_str(), callid.length());
      result += callid.length();
      return result;
    }
    void fromBuffer(char* buffer, int length) {
		int offset = 0;
		command = buffer[offset ++];
		payload = buffer[offset ++];
		code = buffer[offset ++];
        slot = buffer[offset ++];
		memcpy(&addr, buffer + offset, sizeof(sockaddr_in));
		offset += sizeof(sockaddr_in);
		callid = string(buffer + offset, length - offset);
    }
	void print() {
		switch(command) {
			case CONTROL_ADD: cout<<"Add: ";
							  break;
			case CONTROL_REMOVE: cout<<"Remove: ";
								 break;
			default: cout<<"Unknow: ";
		}
		switch(code) {
			case CONTROL_AMR: cout<<"AMR ";
							  break;
			case CONTROL_GSM: cout<<"GSM ";
							  break;
			default: cout<<"Unknow ";
		}
		cout<<to_string(payload)<<inet_ntoa(addr.sin_addr)<<":"<<to_string(htons(addr.sin_port))<<" "<<callid<<endl;
	}
};
#endif
