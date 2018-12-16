#include <pthread.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <string>
#include <regex>
#include <map>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "rtp.hpp"
#include "controlpacket.hpp"
#include "Amr/coder.h"
#include "srsuecontrolpacket.hpp"

#include "stopwatch.h"

#define BUFFER_SIZE 10240

#define TYPE_ABORT 0x00
#define TYPE_NOCHANGE 0x01
#define TYPE_GSMAMR 0x02
#define TYPE_AMRGSM 0x03
using namespace std;

static int listenPort;
static int controlPort;
static string listenIP;
static uint8_t packetBuffer[64];

struct SrcInfo {
	uint8_t payload;
	uint8_t codetype;
    uint8_t slot;
	void* coder;
};

struct TransConf {
    uint8_t type;
    uint8_t tarPT;
    sockaddr_in tarAddr;
	void* coder;
};

struct addrSlots{
    sockaddr_in addrs[4];
    sockaddr_in& operator [](int i) {
        if(i > 3)
            i = 3;
        else if(i < 0)
            i = 0;
        return *&addr[i];
    }
}

struct addrComp { 
    bool operator () (const sockaddr_in& a1, const sockaddr_in& a2) const {
		int temp = memcmp(&a1.sin_addr, &a2.sin_addr, sizeof(in_addr));
		return temp < 0 || (temp == 0 && a1.sin_port < a2.sin_port);
    }
};

map<sockaddr_in, SrcInfo, addrComp> srcMap;
map<sockaddr_in, sockaddr_in, addrComp> rtpMap;
map<sockaddr_in, sockaddr_in, addrComp> rtcpMap;
map<string, addrSlots> callidMap;
/*
 * Addr helper
 *
 *
 * */

static sockaddr_in str2addr(string IP, int port) {
	sockaddr_in result;
	result.sin_family = AF_INET;
	result.sin_port = htons(port);
    inet_pton(AF_INET, IP.c_str(), &result.sin_addr);
	return result;
}

static string addr2str(sockaddr_in addr) {
	string result = inet_ntoa(addr.sin_addr);
	result = result + ":" + to_string(ntohs(addr.sin_port));
	return result;
}

static TransConf getConf(const sockaddr_in& addr) {
	//cout<<"Receive From: "<<addr2str(addr)<<endl;
    TransConf conf;
    map<sockaddr_in, sockaddr_in>::iterator it = rtpMap.find(addr);
    if(it == rtpMap.end()) {
        conf.type = TYPE_ABORT;
        return conf;
    }
    SrcInfo srcInfo = srcMap[addr];
    SrcInfo tarInfo = srcMap[it->second];
    if(srcInfo.codetype == tarInfo.codetype)
        conf.type = TYPE_NOCHANGE;
	else if(srcInfo.codetype == CONTROL_GSM)
		conf.type = TYPE_GSMAMR;
	else
		conf.type = TYPE_AMRGSM;
    conf.tarPT = tarInfo.payload;
    conf.tarAddr = it->second;
	conf.coder = srcInfo.coder;
    return conf;
}

static void sendBackConnect(sockaddr_in addr) {
    uint16_t thisPort = htons(listenPort);
    srsueControlPacket packet;
    packet.event = SRSUE_ADD_ADDPORT;
    memcpy(&packet.data[0], addr.sin_port, 2);
    memcpy(&packet.data[2], &thisPort, 2);
    addr.sin_port = htons(5080);
    srsueControlPacketToBuffer(packet, &packetBuffer[0]);
    sendto()
    thisPort =  htons()
    //TODO  info srsue
}


static void addSrc(const ControlPacket& packet) {
    SrcInfo info = {packet.payload, packet.code, packet.slot, NULL};
    srcMap[packet.addr] = info;
    map<string, sockaddr_in>::iterator it = callidMap.find(packet.callid);
    callidMap[packet.callid][packet.slot] = packet.addr;
    if(packet.slot & 0x01 == 0x01) {
        sockaddr_in srcAddr = packet.addr;
		sockaddr_in tarAddr = callidMap[packet.callid][packet.slot ^ 0x01];
		srcIt = srcMap.find(srcAddr);
		tarIt = srcMap.find(tarAddr);
		if(srcIt->second.codetype == CONTROL_AMR && tarIt->second.codetype == CONTROL_GSM) {
			srcIt->second.coder = new AmrToGsmCoder();
			tarIt->second.coder = new GsmToAmrCoder();
            sendBackConnect(srcAddr);
		}
		else if(srcIt->second.codetype == CONTROL_GSM && tarIt->second.codetype == CONTROL_AMR) {
			srcIt->second.coder = new GsmToAmrCoder();
			tarIt->second.coder = new AmrToGsmCoder();
            sendBackConnect(tarAddr);
		}
        rtpMap[srcAddr] = tarAddr;
        rtpMap[tarAddr] = srcAddr;
		srcAddr.sin_port = htons(ntohs(srcAddr.sin_port) + 1);
		tarAddr.sin_port = htons(ntohs(tarAddr.sin_port) + 1);
		rtcpMap[srcAddr] = tarAddr;
		rtcpMap[tarAddr] = srcAddr;
    }
}

static void removeCall(const string& callid) {
    map<string, sockaddr_in>::iterator it = callidMap.find(callid);
    if(it == callidMap.end()) {
        return;
    }
    srcMap.erase(srcMap.find(it->second));
    map<sockaddr_in, sockaddr_in>::iterator addrit = rtpMap.find(it->second);
    if(addrit == rtpMap.end())
        return;
    srcMap.erase(srcMap.find(addrit->second));
    rtpMap.erase(rtpMap.find(addrit->second));
    rtpMap.erase(addrit);
}

/*
 *
 * Threads
 *
 * */

static void* RTPControlThread(void* input) {
    // TODO socket_fd read timeout
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        cout<<"Error SIP Socket"<<endl<<"Exit"<<endl;
        return NULL;
    }
    sockaddr_in listenAddr = str2addr(listenIP, controlPort);
	socklen_t addrSize = sizeof(sockaddr_in);
	ControlPacket packet;
    uint8_t receiveBuffer[BUFFER_SIZE];
	int err;
	int count;
	if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr)) != 0) {
		cout<<"Wrong while binding control"<<endl;
		return NULL;
	}
	while(true) {
		count = (int)recv(socket_fd, receiveBuffer, BUFFER_SIZE, 0);
        if(count > 0) {
            packet.fromBuffer((char*)receiveBuffer, count);
			packet.print();
            switch(packet.command) {
                case CONTROL_ADD:
                    addSrc(packet);
                    // TODO we should send srsueControlPacket here
                    break;
                case CONTROL_REMOVE:
                    removeCall(packet.callid);
                    break;
                default:
                    cout<<"Unknow Command"<<endl;
            }
        }
	}
    return NULL;
}

static void* RTPThread(void* input) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        cout<<"Error RTP Socket"<<endl<<"Exit"<<endl;
        return NULL;
    }
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t outputBuffer[BUFFER_SIZE];
	uint8_t codeBuffer[BUFFER_SIZE];
    sockaddr_in rtpAddr;
    sockaddr_in tempAddr;
    socklen_t addrSize = sizeof(sockaddr);
    RTPPacket packet;
	int firstFlag = 0;
	int len;
    rtpAddr = str2addr(listenIP, listenPort);
     if(bind(socket_fd, (sockaddr*)&rtpAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error RTP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
     Stopwatch watch(7, 50, "./log/stopwatch");
    while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        if(len <= 0)
            continue;
        watch.reset(0);
        TransConf conf = getConf(tempAddr);
        watch.record(0);
		switch(conf.type) {
			case TYPE_ABORT:
				cout<<"Abort packet"<<endl;
				continue;
			case TYPE_NOCHANGE:
				pharse_raw(receiveBuffer, len, &packet);
				break;
			case TYPE_AMRGSM:
                watch.reset(1);
				pharse_AMR(receiveBuffer, len, &packet);
				packet.extLen = 0;
                watch.record(1);
                watch.reset(2);
				((AmrToGsmCoder*)conf.coder)->AmrToGsm(packet.buffer, &codeBuffer[0]);
				packet.buffer = &codeBuffer[0];
				packet.len = 33; 
                watch.record(2);
				break;
			case TYPE_GSMAMR:
                watch.reset(3);
				pharse_GSM(receiveBuffer, len, &packet);
				packet.extLen = 1;
                watch.record(3);
                watch.reset(4);
				packet.len = ((GsmToAmrCoder*)conf.coder)->GsmToAmr(packet.buffer, &codeBuffer[0]);
				packet.buffer = &codeBuffer[0];
				packet.extHead[0] = (((codeBuffer[0] >> 3) & 0x0F) << 4) & 0xF0;
                watch.record(4);
				break;
			default:
				continue;
		}
        watch.reset(5);
        packet.setPT(conf.tarPT);
        len = rtp2buffer(outputBuffer, &packet);
        watch.record(5);
		if(firstFlag == 0) {
			usleep(40000);
			firstFlag = 1;
		}
        len = sendto(socket_fd, outputBuffer, len, 0, (sockaddr*)&conf.tarAddr, sizeof(sockaddr));
		//cout<<"Sent length: "<<len<<endl;
    }
    return NULL;
}
static void* RTCPThread(void* input) {
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        cout<<"Error RTCP Socket"<<endl<<"Exit"<<endl;
        return NULL;
    }
    uint8_t receiveBuffer[BUFFER_SIZE];
    sockaddr_in rtcpAddr;
    sockaddr_in tempAddr;
    sockaddr_in tarAddr;
    socklen_t addrSize = sizeof(sockaddr);
    char buffer_test[20];
    map<sockaddr_in, sockaddr_in, addrComp>::iterator it;
    ssize_t len;
    rtcpAddr = str2addr(listenIP, listenPort + 1);
     if(bind(socket_fd, (sockaddr*)&rtcpAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error RTCP bind"<<endl<<"Exit"<<endl;
        perror("In RTCP:");
        return NULL;
    }
     while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        /*if(getRTCPTarAddr(tempAddr, tarAddr) == 0) {
            sendto(socket_fd, receiveBuffer, len, 0, (sockaddr*)&tarAddr, sizeof(sockaddr));
        }*/
		if(rtcpMap.count(tempAddr) != 0) {
			sendto(socket_fd, receiveBuffer, len, 0, (sockaddr*)&rtcpMap[tempAddr], sizeof(sockaddr));
		}
     }
}

int main(int argc, char** argv) {
    if(argc != 4) {
        cout<<"Wrong argv: ListenIP ListenPort controlPort "<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
	controlPort = atoi(argv[3]);
	string buffer;
    pthread_t RTPid;
	pthread_t RTCPid;
	pthread_t RTPControlid;
    pthread_create(&RTPid, NULL, &RTPThread, NULL);
	pthread_create(&RTCPid, NULL, &RTCPThread, NULL);
	pthread_create(&RTPControlid, NULL, &RTPControlThread, NULL);
    while(cin>>buffer) {
        if(buffer.compare("quit") == 0) {
            break;
        }
    }
    return 0;
}
