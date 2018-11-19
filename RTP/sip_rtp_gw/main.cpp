#include <pthread.h>
#include <string>
#include <regex>
#include <map>
#include <rtpsession.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include <rtpsession.h>
#include <rtpsessionparams.h>
#include <rtpudpv4transmitter.h>
#include <rtpsourcedata.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 10240

using namespace std;
struct user {
    sockaddr_in *saddr;
    sockaddr_in *raddr;
    user* speakTo;
    int rfd;// RTP socket fd
    int rtfd;// RTCP socket fd
};

static map<sring, user> users;
static int listenPort;
static int serverPort;
static int offsetPort;
static string listenIP;
static string serverIP;

static string getText(const string& patten, const string& target) {
    smatch sm;
    if(regex_search(target.cbegin(), target.cend(), sm, regex(patten)))
        return sm.str();
    else
        return string();
}

static string replaceFirst(const string& patten,const string& src, const string& tar) {
    return regex_replace(src, patten, tar, regex_constants::format_first_only);
}
static string replaceAll(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, patten, tar);
}

static user& getUser(const string& id) {
    if(0 == users.count(id)) {
        user newUser = {NULL, NULL, NULL, -1, -1};
        users[id] = newUser;
    }
    return users[id];
}

static void* SIPThread(void* input) {
    // TODO socket_fd read timeout
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        cout<<"Error SIP Socket"<<endl<<"Exit"<<endl;
        return NULL;
    }
    sockaddr_in listenAddr;
    sockaddr_in tempAddr;
    sockaddr_in serverAddr;
    socklen_t addrSize;
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t sendBuffer[BUFFER_SIZE];

    string fromPatten = "From: <sip:[0-9]+";
    string toPatten = "To: <sip:[0-9]+";
    string sdpLengthPatten = "Content-Length: [0-9]+";
    string viaPatten = "Via: SIP/2.0/UDP [0-9]+\.[0-9]+\.[0-9]+\.[0-9]+:[0-9]+";
    string sdpIPPatten = "IN IP4 [0-9]+\.[0-9]+\.[0-9]+\.[0-9]+";
    string sdpPortPatten = "audio [0-9]+";

    string viaString = "Via: SIP/2.0/UDP " + listenIP + ":" + to_string(listenPort);
    string sdpIPString = "IN IP4 " + listenIP;
    
    listenAddr.sin_family = AF_INET;
    inet_pton(AF_INET, listenIP.c_str(), &listenAddr.sin_addr);
    listenAddr.sin_port = htons(listenPort);
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(serverPort);

    if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error SIP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
    while(true) {
        pthread_testcancel();
        ssize_t len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        if(len >= 0) {
            string receiveString((char*)&receiveBuffer[0], (int)len);
            string fromID = getText(fromPatten, receiveString);
            string toID = getText(toPatten, receiveString);
            string sdpLength = getText(sdpLengthPatten, receiveString);
            if(fromID.length()) {
                fromID = fromID.substr(11);
                toID = toID.substr(9);
                sdpLength = sdpLength.substr(18);
                cout<<"From:"<<fromID<<" To:"<<toID<<endl;
                
                receiveString = replaceFirst(viaPatten, receiveString, viaString);

                if(!sdpLength.compare("0")) {
                    user fromUser = getUser(fromID);
                    string fromRIP = getText(sdpIPPatten, receiveString).substr(7);
                    string fromRPort = getText(sdpIPPatten, receiveString).substr(6);
                    fromUser.raddr = new sockaddr_in;
                    fromUser.raddr->sin_family = AF_INET;
                    fromUser.raddr->sin_port = htons(atoi(fromRPort.c_str()));
                    inet_pton(AF_INET, fromRIP.c_str(), &fromUser.raddr->sin_addr);
                    
                    if(memcpy(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0) { // From server

                                                // TODO rewrite everything after o=
                    }
                    else { // From client
                        // TODO rewrite everything after o=
                    }
                    receiveString = replaceAll(sdpIPPatten, receiveString, sdpIPString);
                }
            }
        }
    }
    return NULL;
}
static void* RTPThread(void* input) {
    while(true) {
        pthread_testcancel();
    }
    return NULL;
}

int main(int argc, char** argv) {
    if(argc != 5) {
        cout<<"Wrong argv: ListenIP ListenPort ServerIP ServerPort"<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
    serverIP = argv[3];
    serverPort = atoi(argv[4]);
    offsetPort = 1;
    string command;
    pthread_t SIPTid;
    pthread_t RTPTid;
    pthread_create(&SIPTid, NULL, &SIPThread, NULL);
    pthread_create(&RTPTid, NULL, &RTPThread, NULL);

    while(cin>>command) {
        if(command.compare("quit")) {
            pthread_cancel(SIPTid);
            pthread_cancel(RTPTid);
            break;
        }
    }
    return 0;
}
