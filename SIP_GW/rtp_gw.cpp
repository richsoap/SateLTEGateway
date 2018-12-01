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

using namespace std;

static int listenPort;
static int controlPort;
static string listenIP;

struct SrcInfo {
	uint8_t payload;
	uint8_t codetype;
}

map<sockaddr, SrcInfo> srcMap;
map<sockaddr, sockaddr> pairMap;
map<string, sockaddr> callidMap;
// TODO new callid -> new callidMap
// exist callid -> new pairMap
// new addr -> new srcMap
// remove callid -> delete allMaps

/*
 * Some regex helper
 *
 *
*/
string sdpPortPatten = "audio [0-9]+";
string numberPatten = "[0-9]+";
string ipPatten = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";


static string getText(const string& patten, const string& target) {
    smatch sm;
    if(regex_search(target.cbegin(), target.cend(), sm, regex(patten, regex_constants::icase)))
        return sm.str();
    else
        return string();
}
static string filtSDP(const string& src) {
	string result = replaceAll("m=.*\r\n", src, "");
	result = replaceAll("a=.*\r\n", src, "");
	return result;
}
static string replaceSDP(const string& src) {
	string result = replaceAll(ipPatten, RTPIP);
	result = replaceAll(sdpPortPatten, sdpPortString);
}

static string replaceFirst(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, regex(patten, regex_constants::icase), tar, regex_constants::format_first_only);
}
static string replaceAll(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, regex(patten, regex_constants::icase), tar);
}
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

/*
 *
 * SIP Thread
 *
 * */

static void* RTPControlThread(void* input) {
    // TODO socket_fd read timeout
	// add sockaddr pair to map
	// add srcinfo to map
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        cout<<"Error SIP Socket"<<endl<<"Exit"<<endl;
        return NULL;
    }
    sockaddr_in listenAddr = str2addr(listenIP, controlPort);
	socklen_t addrSize = sizeof(sockaddr_in);
	ControlPacketi* packet;
    uint8_t receiveBuffer[BUFFER_SIZE];
	int err;
	int count;
	if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr)) != 0) {
		cout<<"Wrong while binding control"<<endl;
		return;
	}
	while(true) {
		count = (int)recv(socket_fd, receiveBuffer, BUFFER_SIZE, 0);

	}
    return NULL;
}

static void* RTPThread(void* input) {
	// TODO depend on srcinfo to decode and encode
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        cout<<"Error RTP Socket"<<endl<<"Exit"<<endl;
        return NULL;
    }
    uint8_t receiveBuffer[BUFFER_SIZE];
    sockaddr_in rtpAddr;
    sockaddr_in tempAddr;
    socklen_t addrSize;
    char buffer_test[20];
    map<sockaddr_in, sockaddr_in, addrComp>::iterator it;
    ssize_t len;
    rtpAddr.sin_family = AF_INET;
    inet_pton(AF_INET, listenIP.c_str(), &rtpAddr.sin_addr);
    rtpAddr.sin_port = htons(rtpPort);
     if(bind(socket_fd, (sockaddr*)&rtpAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error RTP bind"<<endl<<"Exit"<<endl;
	perror("In RTP:");
        return NULL;
    }
    while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
	    //cout<<"Temp addr: "<<addrSize<<" :"<< buffer_test<<":"<<ntohs(tempAddr.sin_port)<<endl;
        switch(getPT(receiveBuffer)) {
            case 118:setPT(receiveBuffer, 99);cout<<"[in PT 118(client)\n";
                     break;
            case 99:setPT(receiveBuffer, 118);cout<<"[in PT 99(phone)\n";
                     break;
            default:
                     cout<<"Unknow PT:"<<getPT(receiveBuffer)<<endl;
        }
        if((it = rtpMap.find(tempAddr)) != rtpMap.end()) {
            printAddr(tempAddr);
            printAddr(it->second);
            len = sendto(socket_fd, receiveBuffer, len, 0, (sockaddr*)&it->second, sizeof(sockaddr));
            cout<<"Transmit size: "<<len<<endl;
        }
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
    socklen_t addrSize;
    char buffer_test[20];
    map<sockaddr_in, sockaddr_in, addrComp>::iterator it;
    ssize_t len;
    rtcpAddr.sin_family = AF_INET;
    inet_pton(AF_INET, listenIP.c_str(), &rtcpAddr.sin_addr);
    rtcpAddr.sin_port = htons(rtpPort + 1);
     if(bind(socket_fd, (sockaddr*)&rtcpAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error RTCP bind"<<endl<<"Exit"<<endl;
        perror("In RTCP:");
        return NULL;
    }
     while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
	    inet_ntop(AF_INET, &tempAddr.sin_addr, buffer_test, 20);
	    //cout<<"Temp addr: "<<addrSize<<" :"<< buffer_test<<":"<<ntohs(tempAddr.sin_port)<<endl;
        addr2rtp(tempAddr);
        if((it = rtpMap.find(tempAddr)) != rtpMap.end()) {
            tempAddr = it->second;
            addr2rtcp(tempAddr);
            len = sendto(socket_fd, receiveBuffer, len, 0, (sockaddr*)&tempAddr, sizeof(sockaddr));
            cout<<"RTCP transmit size: "<<len<<endl;
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
