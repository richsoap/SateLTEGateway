#include <pthread.h>
#include <osip_message.h>
#include <osip_parser.h>
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

#define BUFFER_SIZE 10240

#define TYPE_ABORT 0x00
#define TYPE_NOCHANGE 0x01
#define TYPE_CHANGE
using namespace std;

static int listenPort;
static int controlPort;
static string listenIP;

struct SrcInfo {
	uint8_t payload;
	uint8_t codetype;
}

struct TransConf {
    uint8_t type;
    uint8_t tarPT;
    uint8_t srcCode;
    uint8_t tarCode;
    sockaddr tarAddr;
}

map<sockaddr, SrcInfo> srcMap;
map<sockaddr, sockaddr> pairMap;
map<string, sockaddr> callidMap;
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

static int getRTCPAddr(sockaddr_in srcAddr, sockaddr_in& tarAddr) {
    srcAddr.sin_port = htons(ntohs(srcAddr.sin_port) - 1);
    if(pairMap.count(srcAddr) == 0)
        return -1;
    tarAddr = pairMap[srcAddr];
    tarAddr.sin_port = htons(ntohs(tarAddr.sin_port) + 1);
}

static TransConf getConf(const sockaddr& addr) {
    TransConf conf;
    map<sockaddr, sockaddr>::iterator it = pairMap.find(addr);
    if(it == pairMap.end()) {
        conf.type = TYPE_ABORT;
        return conf;
    }
    SrcInfo srcInfo = srcMap[addr];
    SrcInfo tarInfo = srcMap[it->second];
    if(srcInfo.codetype == tarInfo.codetype)
        conf.type = TYPE_NOCHANGE;
    else {
        conf.type = TYPE_CHANGE;
    }
    conf.tarPT = tarInfo.payload;
    conf.srcCode = srcInfo.code;
    conf.tarCode = tarInfo.code;
    conf.tarAddr = tarInfo.addr;
    return conf;
}

static void addSrc(const ControlPacket& packet) {
    SrcInfo info = {packet.payload, packet.code};
    srcMap[packet.addr] = info;
    map<string, sockaddr>::interator it = callidMap.find(packet.callid);
    if(it == callidMap.end()) {
        callidMap[packet.callid] = packet.addr;
    }
    else {
        pairMap[it->second] = packet.addr;
        pariMap[packet.addr] = it->second;
    }
}

static void removeCall(const string& callid) {
    map<string, sockaddr>::iterator it = callidMap.find(callid);
    if(it == callidMap.end()) {
        return;
    }
    srcMap.erase(srcMap.find(it->second));
    map<sockaddr, sockaddr>::iterator addrit = pairMap.find(it->second);
    if(addrit == pairMap.end())
        return;
    srcMap.erase(srcMap.find(addrit->second));
    pairMap.erase(pairMap.find(addrit->second));
    pairMap.erase(addrit);
}

/*
 *
 * SIP Thread
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
	ControlPacketi packet;
    uint8_t receiveBuffer[BUFFER_SIZE];
	int err;
	int count;
	if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr)) != 0) {
		cout<<"Wrong while binding control"<<endl;
		return;
	}
	while(true) {
		count = (int)recv(socket_fd, receiveBuffer, BUFFER_SIZE, 0);
        if(count > 0) {
            packet.fromBuffer(receiveBuffer, count);
            switch(packet.command) {
                case CONTROL_ADD:
                    addSrc(packet);
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
    sockaddr_in rtpAddr;
    sockaddr_in tempAddr;
    socklen_t addrSize = sizeof(sockaddr);
    RTPPacket packet;
    rtpAddr = str2addr(listenIP, listenPort);
     if(bind(socket_fd, (sockaddr*)&rtpAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error RTP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
    while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        if(len <= 0)
            continue;
        TransConf conf = getConf(tempAddr);
        if(conf.type == TYPE_ABORT)
            continue;
        else if(conf.type == TYPE_CHANGE) {
            switch(conf.srcCode) {
                case CONTROL_GSM:
                    pharse_gsm(receiveBuffer, len, &packet);
                    //TODO GSM to PCM
                    break;
                case CONTROL_AMR:
                    //TODO pharse_PCM(receiveBuffer, len, &packet);
                    // AMR to PCM
                    break;
                default:
                    break;
            }
            switch(conf.tarCode) {
                case CONTROL_GSM:
                    //TODO encode GSM
                    break;
                case CONTROL_AMR:
                    //TODO encode AMR
                    // Add extHead
                    break;
                default:
                    break;
            }
        }
        packet->setPT(conf.tarPT);
        len = rtp2buffer(outputBuffer, packet);
        sendto(socket_fd, outputBuffer, len, 0, (sockaddr*)&conf.tarAddr, sizeof(sockaddr));
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
        if(getRTCPTarAddr(tempAddr, &tarAddr) == 0) {
            sendto(socket_fd, receiveBuffer, len, 0, (sockaddr*)&tarAddr, sizeof(sockaddr));
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
    pthread_t RTPid;
	pthread_t RTPCid;
	pthread_t RTPControlid;
    pthread_create(&RTPid, NULL, &RTPThread, NULL);
	pthread_create(&RTPCid, NULL, &RTPCThread, NULL);
	pthread_create(&RTPControlid, NULL, &RTPControlid, NULL);
    while(cin>>buffer) {
        if(command.compare("quit") == 0) {
            break;
        }
    }
    return 0;
}
