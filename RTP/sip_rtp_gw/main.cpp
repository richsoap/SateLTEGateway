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
    string text = getText("SIP/2.0 [0-9]{3}", src);
    cout<<"In isStateCode: "<<text<<endl;
    return text.length() != 0;
}
static string replaceMedia(const string& src, const string& tar) {
    smatch sm;
    if(regex_search(src.cbegin(), src.cend(), sm, regex("m=", regex_constants::icase)))
        return string(sm.prefix()) + string(sm.str()) + tar;
    else
        return src;
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
	cout<<"Add new user"<<id<<endl;
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
* RTP Helper
*
*/

static uint8_t getPT(uint8_t* buffer) {
    return buffer[1] & 0x7f;
}
static void setPT(uint8_t* buffer, uint8_t PT) {
    buffer[1] = (buffer[1] & 0x80) | PT;
}

static void printAddr(const sockaddr_in& addr) {
	char buffer_test[20];
	inet_ntop(AF_INET, &addr.sin_addr, buffer_test, 20);
        cout<< buffer_test<<":"<<ntohs(addr.sin_port)<<endl;
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

    string fromPatten = "(f|From):.*<(sip|tel):[0-9]+";
    string toPatten = "(t|To):.*<(sip|tel):[0-9]+";
    string sdpLengthPatten = "(Content-Length|l):\\s+[0-9]+";
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
    string sdpToClientString = "audio 5062 RTP/AVP 118\r\na=sendrecv\r\na=rtpmap:118 AMR-WB/16000/1\r\na=ptim:20\r\na=maxptime:240\r\n";

    string sdpToServerString = "audio 50010 RTP/AVP 99\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\n";
    //string sdpToServerString = "audio 50010 RTP/AVP 99\r\nb=AS:49\r\nb=RS:600\r\nb=RR:2000\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 mode-change-capability=2;max-red=0\r\na=curr:qos local none\r\na=curr:qos remote none\r\na=des:qos mandatory local sendrecv\r\na=des:qos optional remote sendrecv\r\na=sendrecv\r\na=maxptime:240\r\na=ptime:20\r\n";
    listenAddr.sin_family = AF_INET;
    inet_pton(AF_INET, listenIP.c_str(), &listenAddr.sin_addr);
    listenAddr.sin_port = htons(listenPort);
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(serverPort);

    servers["001010000000001"] = {serverAddr, "192.168.4.1"};
    servers["001010000000003"] = {serverAddr, "192.168.4.1"};
    if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error SIP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
    while(true) {
        pthread_testcancel();
        ssize_t len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);

        cout<<"Temp addr: ";
printAddr(tempAddr);
cout<<(int)len<<endl;
        if(len >= 0) {
            string receiveString((char*)&receiveBuffer[0], (int)len);
            string fromID = getText(fromPatten, receiveString);
            string toID = getText(toPatten, receiveString);
            string sdpLength = getText(sdpLengthPatten, receiveString);
cout<<"raw sdpLength: "<<sdpLength<<" Length: "<<sdpLength.length()<<endl;
            //cout<<fromID<<endl;
            if(fromID.length()) {
cout<<receiveString;
                fromID = getText(numberPatten, fromID);
                toID = getText(numberPatten, toID);
                sdpLength = getText(numberPatten, sdpLength);
                //cout<<"From:"<<fromID<<" To:"<<toID<<" sdp:"<<sdpLength<<endl;
		//cout<<fromID<<endl;
                receiveString = replaceFirst(viaPatten, receiveString, viaString);
// Add SIP Addr
                user* fromUser = getUser(fromID);
		        if(!isFromServer(fromID, tempAddr) && (isContain(regesterPatten, receiveString) || fromUser->saddr == NULL) && ntohs(tempAddr.sin_port) != 0) {
                    fromUser->saddr = new sockaddr_in;
		            memcpy(fromUser->saddr, &tempAddr, sizeof(sockaddr));
		            cout<<"From Saddr: ";
printAddr(*fromUser->saddr);
		        }
cout<<"Finish add sip addr"<<endl;
// Add SDP Addr
                string tarID = isStateCode(receiveString)?fromID:toID;
                string srcID = isStateCode(receiveString)?toID:fromID;
                if(atoi(sdpLength.c_str()) > 0 && ntohs(tempAddr.sin_port) == 5060 /*&& isContain(INVITEPatten, receiveString)*/) {
                    string fromRIP = getText(ipPatten, getText(sdpIPPatten, receiveString));
                    string fromRPort = getText(numberPatten, getText(sdpPortPatten, receiveString));
cout<<"***************RIP:"<<fromRIP<<" "<<fromRPort<<" "<<isStateCode(receiveString)<<endl;
		    user* srcUser = getUser(srcID);
		    user* tarUser = getUser(tarID);
                    if(srcUser->raddr != NULL)
                        delete srcUser->raddr;
		    srcUser->raddr = new sockaddr_in;
		    memset(srcUser->raddr, 0, sizeof(sockaddr));
                    srcUser->raddr->sin_family = AF_INET;
                    srcUser->raddr->sin_port = htons(atoi(fromRPort.c_str()));
                    inet_pton(AF_INET, fromRIP.c_str(), &srcUser->raddr->sin_addr);
		cout<<srcID<<": ";
                    printAddr(*srcUser->raddr);
cout<<"--------------------"<<endl;
                    if(isContain(OKPatten, receiveString)) {
                        rtpMap[*srcUser->raddr] = *tarUser->raddr;
                        rtpMap[*tarUser->raddr] = *srcUser->raddr;
			cout<<"Add rtp con ";
			printAddr(rtpMap[*srcUser->raddr]);
			cout<<" ";
			printAddr(rtpMap[*tarUser->raddr]);
			cout<<endl;

                    }
                    cout<<"Tar ID: "<<tarID<<" is from server: "<<isFromServer(tarID, tempAddr)<<endl;
                    if(isFromServer(tarID, tempAddr)) { // From server
			//cout<<"***************** Change SDP to client\n";
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
		        if(isFromServer(tarID, tempAddr)) {
			    //cout<<"In send to user"<<endl;
		            if(getUser(tarID)->saddr != NULL) {
		                ssize_t result = sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)(getUser(tarID)->saddr), sizeof(sockaddr));
		                inet_ntop(AF_INET, &getUser(tarID)->saddr->sin_addr, buffer_test, 20);
		                //cout<<"Saddr: "<< buffer_test<<":"<<ntohs(getUser(toID)->saddr->sin_port)<<endl;
		                //cout<<"Send to user "<<tarID<<" "<<getUser(tarID)->saddr<<" "<<(int)result<<endl;
		            }
		        }
		        else {
                           //cout<<"In send to server"<<endl;
                    if(servers.count(tarID)) {
		                sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)&servers[tarID], sizeof(sockaddr));
				//sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)(getUser(fromID)->saddr), sizeof(sockaddr));
                    }
                    //else
                           //cout<<"Not find serverAddr"<<endl;
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
    if(argc != 5) {
        cout<<"Wrong argv: ListenIP ListenPort ServerIP ServerPort"<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
    rtpPort = 10020;
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
                servers[imsi] = {addr, IP};
            }
            else {
                cout<<"Add command need IP and IMSI"<<endl;
            }
        }
        
    }
    return 0;
}
