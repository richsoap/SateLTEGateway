#include <pthread.h>
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
struct user {
    sockaddr_in *saddr;
    sockaddr_in *raddr;
    user* speakTo;
};

struct server {
    sockaddr_in addr;
    string ip;
};

struct addrComp {
    bool operator() (const sockaddr_in& a, const sockaddr_in& b) const {
        return memcmp(&a, &b, sizeof(sockaddr_in)) < 0;
    }
};

static map<string, user> users;
static map<string, server> servers;
static map<sockaddr_in, sockaddr_in, addrComp> rtpMap;
static int listenPort;
static int rtpPort;
static int serverPort;
static int offsetPort;
static string listenIP;
static string serverIP;
/*
 * Some regex helper
 *
 *
*/
static string getText(const string& patten, const string& target) {
    smatch sm;
    if(regex_search(target.cbegin(), target.cend(), sm, regex(patten, regex_constants::icase)))
        return sm.str();
    else
        return string();
}

static bool isFromServer(const string& id, const sockaddr_in& addr) {
    if(servers.count(id) != 0)
        return memcmp(&servers[id], &addr, sizeof(sockaddr_in)) == 0;
    else
        return false;
}

static string replaceFirst(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, regex(patten, regex_constants::icase), tar, regex_constants::format_first_only);
}
static string replaceAll(const string& patten, const string& src, const string& tar) {
    return regex_replace(src, regex(patten, regex_constants::icase), tar);
}
static bool isContain(const string& patten, const string& src) {
    return getText(patten, src).length() != 0;
}
static bool isStateCode(const string& src) {
    return getText(" [0-9]{3} ", src).length() != 0;
}
static string replaceMedia(const string& src, const string& tar) {


}

static string refreshSdpLength(const string& src) {
    string patten = "(l|Content-Length): [0-9]+";
    smatch sm;
    regex_search(src.cbegin(), src.cend(), sm , regex("\r\n\r\n", regex_constants::icase));
    return replaceFirst(patten, src, "Content-Length: " + to_string(sm.suffix().length()));
}
/*
 * User Map helpter
 *
 *
 * */
static user* getUser(const string& id) {
    if(0 == users.count(id)) {
	cout<<"Add new user\n";
        user newUser = {NULL, NULL, NULL};
        users[id] = newUser;
    }
    return &users.find(id)->second;
}

static void addr2rtp(sockaddr_in& addr) {
    addr.sin_port = htons(ntohs(addr.sin_port) - 1);
}
static void addr2rtcp(sockaddr_in& addr) {
    addr.sin_port = htons(ntohs(addr.sin_port) + 1);
}
/*
 *
 * SIP Thread
 *
 * */

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
    socklen_t addrSize = sizeof(sockaddr_in);
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t sendBuffer[BUFFER_SIZE];

    char buffer_test[1000];

    string fromPatten = "(f|From):.*<sip:[0-9]+";
    string toPatten = "(t|To):.*<sip:[0-9]+";
    string sdpLengthPatten = "(Content-Length|l): [0-9]+";
    string viaPatten = "SIP/2.0/UDP [0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+:[0-9]+";
    string sdpIPPatten = "IN IP4 [0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
    string sdpPortPatten = "audio [0-9]+";
    string INVITEPatten = "Cseq: [0-9]+ INVITE";
    string regesterPatten = "Cseq: [0-9]+ REGISTER";
    string OKPatten = "200 OK";
    string numberPatten = "[0-9]+";
    string ipPatten = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";

    string viaString = "SIP/2.0/UDP " + listenIP + ":" + to_string(listenPort);
    string sdpIPString = "IN IP4 " + listenIP;
    string sdpPortString = "audio " + to_string(rtpPort);
    string sdpToClientString = "";
    string sdpToServerString = "";

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
	//inet_ntop(AF_INET, &tempAddr.sin_addr, buffer_test, 20);
        //cout<<"Temp addr: "<<addrSize<<" :"<< buffer_test<<":"<<ntohs(tempAddr.sin_port)<<":"<<(int)len<<endl;
        if(len >= 0) {
            string receiveString((char*)&receiveBuffer[0], (int)len);
            string fromID = getText(fromPatten, receiveString);
            string toID = getText(toPatten, receiveString);
            string sdpLength = getText(sdpLengthPatten, receiveString);
            cout<<fromID<<endl;
            if(fromID.length()) {
                fromID = getText(numberPatten, fromID);
                toID = getText(numberPatten, toID);
                sdpLength = getText(numberPatten, sdpLength);
                //cout<<"From:"<<fromID<<" To:"<<toID<<" sdp:"<<sdpLength<<endl;
		cout<<fromID<<endl;
                receiveString = replaceFirst(viaPatten, receiveString, viaString);
// Add SIP Addr
                user* fromUser = getUser(fromID);
		        if(!isFromServer(fromID, tempAddr) && (isContain(regesterPatten, receiveString) || fromUser->saddr == NULL) && ntohs(tempAddr.sin_port) != 0) {
                    fromUser->saddr = new sockaddr_in;
		            memcpy(fromUser->saddr, &tempAddr, sizeof(sockaddr));
		            inet_ntop(AF_INET, &getUser(fromID)->saddr->sin_addr, buffer_test, 20);
		            //cout<<"From Saddr: "<< buffer_test<<":"<<ntohs(getUser(fromID)->saddr->sin_port)<<endl;
		        }
// Add SDP Addr
			    string tarID = isStateCode(receiveString)?fromID:toID;
                if(atoi(sdpLength.c_str()) > 0 && isContain(INVITEPatten, receiveString)) {
                    string fromRIP = getText(ipPatten, getText(sdpIPPatten, receiveString));
                    string fromRPort = getText(numberPatten, getText(sdpIPPatten, receiveString));
                    if(fromUser->raddr == NULL)
		                fromUser->raddr = new sockaddr_in;
                    fromUser->raddr->sin_family = AF_INET;
                    fromUser->raddr->sin_port = htons(atoi(fromRPort.c_str()));
                    inet_pton(AF_INET, fromRIP.c_str(), &fromUser->raddr->sin_addr);
                    if(isContain(OKPatten, receiveString)) {
                        rtpMap[*getUser(fromID)->raddr] = rtpMap[*getUser(toID)->raddr];
                        rtpMap[*getUser(toID)->raddr] = rtpMap[*getUser(fromID)->raddr];
                    }
                    if(isFromServer(tarID, tempAddr)) { // From server
                        receiveString = replaceMedia(receiveString, sdpToClientString);
                    }
                    else { // From client
                        receiveString = replaceMedia(receiveString, sdpToServerString);
                    }
                    receiveString = replaceAll(sdpIPPatten, receiveString, sdpIPString);// TODO change to serverIP uplink: ServerIP downlink: listenIP
                    receiveString = replaceFirst(sdpPortPatten, receiveString, sdpPortString);
                    receiveString = refreshSdpLength(receiveString);
                }
// Send SIP packet
			    cout<<"In send to server"<<endl;
		        if(isFromServer(tarID, tempAddr)) {
		            if(getUser(tarID)->saddr != NULL) {
		                ssize_t result = sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)(getUser(tarID)->saddr), sizeof(sockaddr));
		                inet_ntop(AF_INET, &getUser(tarID)->saddr->sin_addr, buffer_test, 20);
		                //cout<<"Saddr: "<< buffer_test<<":"<<ntohs(getUser(toID)->saddr->sin_port)<<endl;
		                cout<<"Send to user "<<tarID<<" "<<getUser(tarID)->saddr<<" "<<(int)result<<endl;
		            }
		        }
		        else {
                    if(servers.count(tarID))
		                sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)&servers[tarID], sizeof(sockaddr));
		        }
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
        return NULL;
    }
    while(true) {
        pthread_testcancel();
        len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
	    inet_ntop(AF_INET, &tempAddr.sin_addr, buffer_test, 20);
	    //cout<<"Temp addr: "<<addrSize<<" :"<< buffer_test<<":"<<ntohs(tempAddr.sin_port)<<endl;
        if((it = rtpMap.find(tempAddr)) != rtpMap.end()) {
            len = sendto(socket_fd, receiveBuffer, len, 0, (sockaddr*)&it->second, sizeof(sockaddr));
            //cout<<"Transmit size: "<<len<<endl;
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
            //cout<<"Transmit size: "<<len<<endl;
        }

     }
}

int main(int argc, char** argv) {
    if(argc != 5) {
        cout<<"Wrong argv: ListenIP ListenPort ServerIP ServerPort"<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
    rtpPort = (listenPort + 10) & 0xfffe;
    serverIP = argv[3];
    serverPort = atoi(argv[4]);
    offsetPort = 1;
    string buffer;
    pthread_t SIPTid;
    pthread_t RTPTid;
    pthread_t RTCPTid;
    pthread_create(&SIPTid, NULL, &SIPThread, NULL);
    pthread_create(&RTPTid, NULL, &RTPThread, NULL);
    pthread_create(&RTCPTid, NULL, &RTCPThread, NULL);
    while(cin>>buffer) {
        string command = getText("[{a-z}+]", buffer);
        if(command.compare("[quit]") == 0) {
            break;
        }
        else if(command.compare("[add]") == 0) {
            string IP = getText("[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+", buffer);
            string imsi = getText("[0-9]{15}", buffer);
            if(IP.length() != 0 && imsi.length() != 0) {
                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(5060);
                inet_pton(AF_INET, IP.c_str(), &addr.sin_addr);
                servers[IP] = {addr, IP};
            }
            else {
                cout<<"Add command need IP and IMSI"<<endl;
            }
        }
        
    }
    return 0;
}
