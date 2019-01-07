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
#include "rtpcontrolpacket.hpp"
#include "Amr/coder.h"
#include "srsuecontrolpacket.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include "stopwatch.h"
#include "commonhelpers.hpp"

#define BUFFER_SIZE 10240

#define TYPE_ABORT 0x00
#define TYPE_NOCHANGE 0x01
#define TYPE_GSMAMR 0x02
#define TYPE_AMRGSM 0x03
using namespace std;
namespace bpo = boost::program_options;

static int listenPort;
static int controlPort;
static string listenIP;
static int srsuePort;
static string configFile;
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
        return addrs[i];
    }
};

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
map<sockaddr_in, int, addrComp> markerStateMap;
/*
 * Addr helper
 *
 *
 * */

static void startParse(int argc, char* argv[]) {
	bpo::options_description common("Configuration options");
	common.add_options()
		("rtp.ip", bpo::value<string>(&listenIP)->default_value("0.0.0.0", "Local host IP"))
		("rtp.control_port", bpo::value<int>(&controlPort)->default_value(5062), "Local control Port")
		("rtp.data_port", bpo::value<int>(&listenPort)->default_value(10020), "RTP Trans Port, RTCP=RTP+1")
		("srsue.control_port", bpo::value<int>(&srsuePort)->default_value(5080), "srsUE Control Port");
	bpo::options_description position("Config file location");
	position.add_options()
		("config_file", bpo::value<string>(&configFile)->default_value("rtp.conf"), "RTP Gateway Config File Location");
	bpo::positional_options_description p;
	p.add("config_file", -1);
	bpo::options_description cmdOptions;
	cmdOptions.add(common).add(position);

	bpo::variables_map varMap;
	bpo::store(bpo::command_line_parser(argc, argv).options(cmdOptions).positional(p).run(), varMap);
	bpo::notify(varMap);
	cout<<"Reading config file "<< configFile<<" ..."<<endl;
	ifstream confReader(configFile.c_str(), ios::in);
	if(confReader.fail()) {
		cout<<"Failed to read"<<endl;
		exit(-1);
	}
	bpo::store(bpo::parse_config_file(confReader, common), varMap);
	bpo::notify(varMap);
	cout<<"rtp.ip="<<listenIP<<endl;
	cout<<"rtp.data_port="<<listenPort<<endl;
	cout<<"rtp.control_port="<<controlPort<<endl;
	cout<<"srsue.control_port="<<srsuePort<<endl;
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

static void addSrc(const RTPControlPacket& packet) {
    SrcInfo info = {packet.payload, packet.code, packet.slot, NULL};
    srcMap[packet.addr] = info;
    map<string, addrSlots>::iterator it = callidMap.find(packet.callid);
    callidMap[packet.callid][packet.slot] = packet.addr;
    if(packet.slot & 0x01 == 0x01) {
        sockaddr_in srcAddr = packet.addr;
		sockaddr_in tarAddr = callidMap[packet.callid][packet.slot ^ 0x01];
		map<sockaddr_in, SrcInfo>::iterator srcIt = srcMap.find(srcAddr);
		map<sockaddr_in, SrcInfo>::iterator tarIt = srcMap.find(tarAddr);
		if(srcIt->second.codetype == CONTROL_AMR && tarIt->second.codetype == CONTROL_GSM) {
			srcIt->second.coder = new AmrToGsmCoder();
			tarIt->second.coder = new GsmToAmrCoder();
		}
		else if(srcIt->second.codetype == CONTROL_GSM && tarIt->second.codetype == CONTROL_AMR) {
			srcIt->second.coder = new GsmToAmrCoder();
			tarIt->second.coder = new AmrToGsmCoder();
		}
        rtpMap[srcAddr] = tarAddr;
        rtpMap[tarAddr] = srcAddr;
		markerStateMap[srcAddr] = 0;
		markerStateMap[tarAddr] = 0;
		srcAddr.sin_port = htons(ntohs(srcAddr.sin_port) + 1);
		tarAddr.sin_port = htons(ntohs(tarAddr.sin_port) + 1);
		rtcpMap[srcAddr] = tarAddr;
		rtcpMap[tarAddr] = srcAddr;
    }
}

static void infoSrsue(int socket_fd, sockaddr_in addr, int bindport, int event) {
	map<sockaddr_in, sockaddr_in>::iterator it = rtpMap.find(addr);
	if(it != rtpMap.end()) {
		sockaddr_in tarAddr = addr;
		tarAddr.sin_port = htons(srsuePort);
		srsueControlPacket packet;
		packet.event = event;
		// Add RTP
		unsigned short int port = htons(bindport);
		memcpy(&packet.data[0], &port,sizeof(port));
		memcpy(&packet.data[2], &addr.sin_port, sizeof(port));
		// Add RTCP
		port = htons(bindport + 1);
		memcpy(&packet.data[4], &port, sizeof(port));
		port = htons(ntohs(addr.sin_port) + 1);
		memcpy(&packet.data[6], &port, sizeof(port));

		int len = srsueControlPacketToBuffer(packet, &packetBuffer[0]);
		sendto(socket_fd, packetBuffer, len, 0, (sockaddr*)&tarAddr, sizeof(sockaddr));
		cout<<"Send to:";
		printAddr(tarAddr);
		tarAddr.sin_addr = it->second.sin_addr;
		memcpy(&packet.data[2], &it->second.sin_port, sizeof(port));
		port = htons(ntohs(it->second.sin_port) + 1);
		memcpy(&packet.data[6], &port, sizeof(port));
		len = srsueControlPacketToBuffer(packet, &packetBuffer[0]);
		sendto(socket_fd, packetBuffer, len, 0, (sockaddr*)&tarAddr, sizeof(sockaddr));
		cout<<"Send to:";
		printAddr(tarAddr);
	}
}

static void removeCall(int socket_fd, const string& callid) {
    map<string, addrSlots>::iterator it = callidMap.find(callid);
    if(it == callidMap.end()) {
        return;
    }
	for(int i = 0;i < 4;i ++) {
		if(srcMap.count(it->second[i]) != 0) {
			infoSrsue(socket_fd, rtpMap[it->second[i]], listenPort, SRSUE_BYE_PORT);
			srcMap.erase(it->second[i]);
			rtpMap.erase(it->second[i]);
			rtcpMap.erase(it->second[i]);
			markerStateMap.erase(it->second[i]);
		}
		
	}
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
	RTPControlPacket packet;
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
					if(packet.slot & 0x01 == 0x01) {
						infoSrsue(socket_fd, packet.addr, listenPort, SRSUE_ADD_PORT);
					}
                    break;
                case CONTROL_REMOVE:
                    removeCall(socket_fd, packet.callid);
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
    // Stopwatch watch(7, 50, "./log/stopwatch");
    while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        if(len <= 0)
            continue;
        TransConf conf = getConf(tempAddr);
		switch(conf.type) {
			case TYPE_ABORT:
				cout<<"Abort packet"<<endl;
				continue;
			case TYPE_NOCHANGE:
				pharse_raw(receiveBuffer, len, &packet);
				break;
			case TYPE_AMRGSM:
                //watch.reset(1);
				pharse_AMR(receiveBuffer, len, &packet);
				packet.extLen = 0;
                //watch.record(1);
                //watch.reset(2);
				((AmrToGsmCoder*)conf.coder)->AmrToGsm(packet.buffer, &codeBuffer[0]);
				packet.buffer = &codeBuffer[0];
				packet.len = 33; 
                //watch.record(2);
				break;
			case TYPE_GSMAMR:
				pharse_GSM(receiveBuffer, len, &packet);
				packet.extLen = 1;
				//packet.extLen = 0;
				packet.len = ((GsmToAmrCoder*)conf.coder)->GsmToAmr(packet.buffer, &codeBuffer[0]);
				if(packet.len == 1) {
					markerStateMap[tempAddr] = 0;
				}
				else if(markerStateMap[tempAddr] == 0){
					markerStateMap[tempAddr] = 1;
					packet.setMarker(1);
				}
				packet.buffer = &codeBuffer[0];
				packet.extHead[0] = (((codeBuffer[0] >> 3) & 0x0F) << 4) & 0xF0;
				break;
			default:
				continue;
		}
        //watch.reset(5);
        packet.setPT(conf.tarPT);
        len = rtp2buffer(outputBuffer, &packet);
        //watch.record(5);
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
	startParse(argc, argv);
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
