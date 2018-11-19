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
using namespace jrtplib;
struct user {
    RTPSession *sess;
    RTPIPv4Address *raddr;
    sockaddr_in *saddr;
    uint32_t timestamp;
    int speakTo;
    int port;
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

static RTPIPv4Address *sockaddr2RTPaddr(const sockaddr_in &saddr) {
    uint8_t ip[] = {0,0,0,0};
    int count = 0;
    char cip[20];
    uint16_t port;
    inet_ntop(AF_INET, &saddr.sin_addr, cip, 20);
    for(int i = 0;i < 20 && cip[i] != '\0';i ++) {
        ip[count] *= 10;
        ip[count] += (cip[i] - '0');
    }
    port = ntohs(saddr.sin_port);
    return new RTPIPv4Address(ip, port);
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
                if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0) {
               //TODO cout<<"user: "<<;
               //Get To ID
               //Change IPs and port in SDP to the sess
               //send this packet
               // TODO when 200OK Bye cut sess down
                }
                else {
                    user newUser;
                    if(0 == users.count(fromID)) {
                        offsetPort += 2;
                        newUser.saddr = new sockaddr_in();
                        memcpy(addr.saddr, &tempAddr, sizeof(sockaddr));
                        users[fromID] = newUser;
                    }
                    newUser = users[fromID];

               // change IPs and port in SDP to the sess
               // or
               // send this packet
                }
                if(sdpLength.compare("0"))
            }
        }
    }
    return NULL;
}
static void* RTPThread(void* input) {
    RTPSession* sess;
    RTPSourceData* src;
    RTPPacket* packet;
    while(true) {
        pthread_testcancel();
        for(map<int, user>::iterator it = users.begin(); it != users.end(); it ++) {
            sess = it->second.sess;
            sess->Poll();
            if(sess->GotoFirstSourceWithData()) {
                do {
                    src = sess->GetCurrentSourceInfo();
                    while(packet = src->GetNextPacket()) {
                        // send this packet to the other user
                        users[it->second.speakTo].sess->SendPacket(packet->GetPacketData(),
                                                            packet->GetPacketLength(),
                                                            packet->GetPayloadType(),
                                                            false,
                                                            packet->GetTimestamp() - it->second.timestamp);
                        it->second.timestamp = packet->GetTimestamp();
                        delete packet;
                    }
                } while(sess->GotoNextSourceWithData());
            }
        }
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
