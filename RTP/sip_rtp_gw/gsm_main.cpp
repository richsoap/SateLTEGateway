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

static map<string, sockaddr_in> branchMap;
static map<sockaddr_in, sockaddr_in, addrComp> rtpMap;
static int listenPort;
static int rtpPort;
static int serverPort;
static int BTSPort;
static string listenIP;
static string serverIP;
static string BTSIP;
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
static string removeFirst(const string& patten, const string& src) {
    smatch sm;
    regex_search(src.cbegin(), src.cend(), sm, regex(patten, regex_constants::icase));
    return string(sm.prefix()) + string(sm.suffix());
}

static string replaceMedia(const string& src, const string& tar) {
    smatch sm;
    if(regex_search(src.cbegin(), src.cend(), sm, regex("\r\nm=", regex_constants::icase)))
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

static string IMSnormalDomain(const string& src) {
    cout<<"Try to Normal Domain"<<endl;
    string fromPatten = "(From|f).*\\s*sip:[a-zA-Z0-9]+@.*>";
    string toPatten = "(To|t).*\\s*sip:[a-zA-Z0-9]+@.*>";

    string fromIMSI = getText(fromPatten, src);
    string toIMSI = getText(toPatten, src);
    cout<<"From IMSI: "<<fromIMSI <<" \nTo IMSI: "<<toIMSI<<endl;
    toIMSI = getText("[0-9]+", toIMSI);
    fromIMSI = getText("[0-9]+", fromIMSI);
    fromIMSI = "001010000000003";
    string result = replaceFirst(fromPatten, src, "From: <sip:" + fromIMSI + "@ims.mnc001.mcc001.3gppnetwork.org>");
    result = replaceFirst(toPatten, result, "To: <sip:" + toIMSI + "@ims.mnc001.mcc001.3gppnetwork.org>");
    return result;
}
static string BTSnormalDomain(const string& src) {
    cout<<"Try to Normal Domain"<<endl;
    string fromPatten = "(From|f).*\\s*sip:[a-zA-Z0-9]+@.*>";
    string toPatten = "(To|t).*\\s*sip:[a-zA-Z0-9]+@.*>";

    string fromIMSI = getText(fromPatten, src);
    string toIMSI = getText(toPatten, src);
    cout<<"From IMSI: "<<fromIMSI <<" \nTo IMSI: "<<toIMSI<<endl;
    toIMSI = getText("[0-9]+", toIMSI);
    fromIMSI = getText("[0-9]+", fromIMSI);
    string result = replaceFirst(fromPatten, src, "From: <sip:" + fromIMSI + "@" + listenIP + ":" + to_string(listenPort) + ">");
    result = replaceFirst(toPatten, result, "To: <sip:" + toIMSI + "@" + listenIP + ":" + to_string(listenPort) + ">");
    return result;
}

/*
 * User Map helpter
 *
 *
 * */
static sockaddr_in str2addr(string ip, string port) {
    sockaddr_in result;
    result.sin_family = AF_INET;
    result.sin_port = htons(atoi(port.c_str()));
    inet_pton(AF_INET, ip.c_str(), &result.sin_addr);
    return result;
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
    sockaddr_in BTSAddr;
    socklen_t addrSize = sizeof(sockaddr_in);
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t sendBuffer[BUFFER_SIZE];

    char buffer_test[1000];

    string viaPatten = "SIP/2.0/UDP [0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+:[0-9]+";
    string sdpIPPatten = "IN IP4 [0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
    string sdpPortPatten = "audio [0-9]+";
    string numberPatten = "[0-9]+";
    string ipPatten = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
    string branchPatten = "branch=[a-zA-Z0-9]+";
    string phyPatten = "P-PHY-Info.*\r\n";
    string IMSIPatten = "IMSI";
    string prePatten = "P-Prefer.*\r\n";
    string accessPatten = "P-Access.*\r\n";
    string forwardsPatten = "Max-Forwards";

    string viaString = "SIP/2.0/UDP " + listenIP + ":" + to_string(listenPort);
    string sdpIPString = "IN IP4 " + listenIP;
    string sdpPortString = "audio " + to_string(rtpPort);
    string sdpToBTSString = "audio 5062 RTP/AVP 3\r\nc=IN IP4 0.0.0.0\r\na=rtpmap:3 GSM/8000\r\n";
    string allowString = "Allow: INVITE,ACK,OPTIONS,BYE,CANCEL,SUBSCRIBE,NOTIFY,REFER,MESSAGE,INFO,PING,PRACK\r\nMax-Forwards";
    string sdpToIMSString = "audio 50010 RTP/AVP 99\r\nc=IN IP4 11.11.11.11\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 octet-align=1;mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\n";
    string qosString = "a=sendrecv\r\na=curr:qos local sendrecv\r\na=curr:qos remote sendrecv\r\na=des:qos mandatory local sendrecv\r\na=des:qos mandatory remote sendrecv\r\n";

    listenAddr.sin_family = AF_INET;
    inet_pton(AF_INET, listenIP.c_str(), &listenAddr.sin_addr);
    listenAddr.sin_port = htons(listenPort);
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
    serverAddr.sin_port = htons(serverPort);
    BTSAddr.sin_family = AF_INET;
    inet_pton(AF_INET, BTSIP.c_str(), &BTSAddr.sin_addr);
    BTSAddr.sin_port = htons(BTSPort);

    if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error SIP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
    while(true) {
        pthread_testcancel();
        ssize_t len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        if(len >= 0) {
            string receiveString((char*)&receiveBuffer[0], (int)len);
            cout<<receiveString;
            receiveString = replaceFirst(viaPatten, receiveString, viaString);
            string fromRIP = getText(ipPatten, getText(sdpIPPatten, receiveString));
            string fromRPort = getText(numberPatten, getText(sdpPortPatten, receiveString));
            receiveString = replaceFirst(ipPatten + ":" + numberPatten +" SIP/2.0", receiveString, "ims.mnc001.mcc001.3gppnetwork.org SIP/2.0");
            receiveString = replaceFirst(forwardsPatten, receiveString, allowString);
            receiveString = removeFirst(phyPatten, receiveString);
            receiveString = removeFirst(prePatten, receiveString);
            receiveString = removeFirst(accessPatten, receiveString);
            receiveString = replaceAll(IMSIPatten, receiveString, "");
            if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0)
                receiveString = BTSnormalDomain(receiveString);
            else 
                receiveString = IMSnormalDomain(receiveString);
            if(fromRIP.length() != 0) {
                string branch = getText(branchPatten, receiveString);
                branch = branch.substr(8);
                cout<<"***************RIP:"<<fromRIP<<" "<<fromRPort<<" "<<branch<<endl;
                auto iter = branchMap.find(branch);
               if(iter == branchMap.end()) {
                    branchMap[branch] = str2addr(fromRIP, fromRPort);
                }
                else {
                    cout<<"******************RIP: connet for: "<<branch<<endl;
                    rtpMap[iter->second] = str2addr(fromRIP, fromRPort);
                    rtpMap[rtpMap[iter->second]] = iter->second;
                    branchMap.erase(iter);
                }
                if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0) {
                    receiveString = BTSnormalDomain(receiveString);
                    receiveString = replaceMedia(receiveString, sdpToBTSString);
                }
                else {
                    receiveString = IMSnormalDomain(receiveString);
                    receiveString = replaceMedia(receiveString, sdpToIMSString);
                }
                receiveString = replaceAll(sdpIPPatten, receiveString, sdpIPString);
                receiveString = replaceAll(sdpPortPatten, receiveString, sdpPortString);
                receiveString = refreshSdpLength(receiveString);
            }
// Send SIP packet
            if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0)
                 sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)&BTSAddr, sizeof(sockaddr));
            else
                 sendto(socket_fd, receiveString.c_str(), receiveString.length(), 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
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
            case 3:setPT(receiveBuffer, 118);cout<<"[in PT 3(client)\n";
                     break;
            case 99:setPT(receiveBuffer, 3);cout<<"[in PT 99(phone)\n";
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
    if(argc != 7) {
        cout<<"Wrong argv: ListenIP ListenPort IMSIP IMSPort BTSIP BTSPort"<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
    rtpPort = 10020;
    serverIP = argv[3];
    serverPort = atoi(argv[4]);
    BTSIP = argv[5];
    BTSPort = atoi(argv[6]);
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
            }
            else {
                cout<<"Add command need IP and IMSI"<<endl;
            }
        }
        
    }
    return 0;
}
