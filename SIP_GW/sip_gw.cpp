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
#include "controlpacket.hpp"

#define BUFFER_SIZE 10240

using namespace std;

static int listenPort;
static int serverPort;
static int clientPort;
static int RTPControlPort;
static int RTPListenPort;
static string listenIP;
static string serverIP;
static string clientIP;
static string RTPIP;
/*
 * Some regex helper
 *
 *
*/
string sdpPortPatten = "audio [0-9]+";
string numberPatten = "[0-9]+";
string ipPatten = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
string AMRPatten = "a=.*AMR-WB.*\r\n";
string GSMPatten = "a=.*GSM.*\r\n";
string callidPatten = "Callid";

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
static string filtSDP(const string& src) {
	string result = replaceAll("m=.*\r\n", src, "");
	result = replaceAll("a=.*\r\n", src, "");
	return result;
}
static string replaceSDP(const string& src, const string& ipString, const string& portString) {
	string result = replaceAll(ipPatten, RTPIP, ipString);
	result = replaceAll(sdpPortPatten, result, portString);
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


static int buildControl(const string& packet, ControlPacket& control) {
    int len;
    string IP = getText(ipPatten, packet);
    string Port = getText(numberPatten, getText(sdpPortPatten, packet));
    control.addr = str2addr(IP, atoi(Port.c_str()));
    string AMRString = getText(AMRPatten, packet);
    string GSMString = getText(GSMPatten, packet);
    control.callid = getText(callidPatten, packet);
    //TODO need to get callid
    string payload;
    control.command = CONTROL_ADD;
    if(AMRString.length() != 0) {
        payload = getText(numberPatten, AMRString);
        control.code = CONTROL_AMR;
    }
    else if(GSMString.length() != 0) {
        payload = getText(numberPatten, GSMString);
        control.code = CONTROL_GSM;
    }
    if(payload.length() == 0) 
        return -1;
    control.payload = atoi(payload.c_str());
    return 0;
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
    sockaddr_in clientAddr;
	sockaddr_in rtpControlAddr;
	socklen_t addrSize = sizeof(sockaddr_in);
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t outputBuffer[BUFFER_SIZE];
	int err;
	int count;
    ControlPacket controlPacket;
	osip_message_t* sip;
	osip_via_t* sip_via;
	osip_content_length_t* sdpLength;
	osip_body_t* sdpBody;
	char* out_buffer;
	char* temp_point;

	listenAddr = str2addr(listenIP, listenPort);
	serverAddr = str2addr(serverIP, serverPort);
	clientAddr = str2addr(clientIP, clientPort);
	rtpControlAddr = str2addr(RTPIP, RTPControlPort);
	string sdpIPString = "IN IP4 " + RTPIP;
	string sdpPortString = "audio " + to_string(RTPListenPort);
	string listenPortStr = to_string(listenPort);
	string sdpToClientString = "audio 5062 RTP/AVP 118\r\na=sendrecv\r\na=rtpmap:118 AMR-WB/16000/1\r\na=fmtp:118 octet-align=1;mode-change-capability=2;max-red=0\r\na=ptime:20\r\na=maxptime:240\r\n";
	string sdpToServerString = "audio 50010 RTP/AVP 99\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 octet-align=1;mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\n";
	string qosString = "a=sendrecv\r\na=curr:qos local sendrecv\r\na=curr:qos remote sendrecv\r\na=des:qos mandatory local sendrecv\r\na=des:qos mandatory remote sendrecv\r\n";

    if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error SIP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
	parser_init();
    while(true) {
        pthread_testcancel();
        ssize_t len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
        if(len >= 0 && (memcmp(&tempAddr, &serverAddr, sizeof(sockaddr) == 0 || memcmp(&tempAddr, &clientAddr, sizeof(sockaddr) == 0)))) {

			osip_message_init(&sip);
			err = osip_message_parse(sip, (char*)receiveBuffer, len);
			if(err != 0) {
				cout<<"Error: failed while parsing!"<<endl;
				continue;
			}

			osip_message_get_via(sip, osip_list_size(&sip->vias)-1, &sip_via);
			osip_free(sip_via->host);
			sip_via->host = (char*)osip_malloc(listenIP.length());
			memcpy(sip_via->host, listenIP.c_str(), listenIP.length());
			osip_free(sip_via->port);
			sip_via->port = (char*)osip_malloc(listenPortStr.length());
			memcpy(sip_via->port, listenPortStr.c_str(), listenPortStr.length());

			//TODO change remove "IMSI" and add Allows
			sdpLength = osip_message_get_content_length(sip);
			if(sdpLength != NULL && atoi(sdpLength->value) > 0) {
				int pos = 0;
				while(!osip_list_eol(&sip->bodies, pos)) {
					sdpBody = (osip_body_t*) osip_list_get(&sip->bodies, pos);
					pos ++;
					string sdp(sdpBody->body);
					if(getText(sdpPortPatten, sdp).length() == 0) {
						osip_free(sdpBody->body);
						sdpBody->length = 0;
						continue;
					}
                    buildControl(sdp, controlPacket);
                    len = controlPacket.toBuffer((char*)outputBuffer);
                    sendto(socket_fd, outputBuffer, len, 0, (sockaddr*)&rtpControlAddr, sizeof(sockaddr));
					sdp = filtSDP(sdp);
					if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0) {
						sdp += sdpToClientString;
					}
					else if(memcmp(&tempAddr, &clientAddr, sizeof(sockaddr)) == 0){
						sdp += sdpToServerString;
					}
					replaceSDP(sdp, sdpIPString, sdpPortString);
					osip_free(sdpBody->body);
					sdpBody->body = (char*)osip_malloc(sdp.length());
					sdpBody->length = sdp.length();
				}
			}
			err = osip_message_to_str(sip, &out_buffer, (size_t*)&count);
			if(memcmp(&tempAddr, &serverAddr, sizeof(sockaddr)) == 0) {
				count = (int)sendto(socket_fd, out_buffer, count, 0, (sockaddr*)&clientAddr, sizeof(sockaddr));
			}
			else {
				count = (int)sendto(socket_fd, out_buffer, count, 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
			}
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if(argc != 10) {
        cout<<"Wrong argv: ListenIP ListenPort ServerIP ServerPort ClientIP ClientPort RTPIP RTPControlPort RTPlistenPort"<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
    serverIP = argv[3];
    serverPort = atoi(argv[4]);
	clientIP = argv[5];
	clientPort = atoi(argv[6]);
	RTPIP = argv[7];
	RTPControlPort = atoi(argv[8]);
	RTPListenPort = atoi(argv[9]);
    string buffer;
    pthread_t SIPTid;
    pthread_create(&SIPTid, NULL, &SIPThread, NULL);
    while(cin>>buffer) {
        if(buffer.compare("quit") == 0) {
            break;
        }
    }
    return 0;
}
