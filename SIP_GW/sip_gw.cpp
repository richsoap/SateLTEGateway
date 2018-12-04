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
#define ADDRSIZE sizeof(sockaddr_in)

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
string mediaPatten = "m=";

static void printAddr(const sockaddr_in& addr) {
	char buffer_test[20];
	inet_ntop(AF_INET, &addr.sin_addr, buffer_test, 20);
	cout<<buffer_test<<":"<<ntohs(addr.sin_port)<<endl;
}

static bool addrCmp(const sockaddr_in& a1, const sockaddr_in& a2) {
	return memcmp(&a1.sin_addr, &a2.sin_addr, sizeof(a1.sin_addr)) == 0 && a1.sin_port == a2.sin_port;
}

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
static string replaceSDP(const string& src, const string& ipString, const string& portString) {
	string result = replaceAll(ipPatten, src, ipString);
	result = replaceAll(sdpPortPatten, result, portString);
	return result;
}
static string replaceMedia(const string& src, const string& media) {
	smatch sm;
	if(regex_search(src.cbegin(), src.cend(), sm, regex(mediaPatten, regex_constants::icase)))
		return string(sm.prefix()) + media;
	else
		return src;
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

static string ALLOWS[] = {"INVITE", "REGISTER", "SUBSCRIBE", "ACK", "CANCEL", "PRACK", "UPDATE","BYE", "MESSAGE"};
static osip_list_t getAllows() {
	osip_list_t result;
	osip_list_init(&result);
	for(int i = 0;i < 9;i ++) {
		osip_allow_t* invite;
		osip_allow_init(&invite);
		invite->value = (char*)osip_malloc(ALLOWS[i].length() + 1);
		strcpy(invite->value, ALLOWS[i].c_str());
		osip_list_add(&result, invite, -1);
	}
	return result;
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
	socklen_t addrSize;
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t outputBuffer[BUFFER_SIZE];
	int err;
	int count;
    ControlPacket controlPacket;
	osip_message_t* sip;
	osip_via_t* sip_via;
	osip_content_length_t* sdpLength;
	osip_body_t* sdpBody;
	osip_contact_t* contact;
	char* out_buffer;
	char* temp_point;

	listenAddr = str2addr(listenIP, listenPort);
	serverAddr = str2addr(serverIP, serverPort);
	clientAddr = str2addr(clientIP, clientPort);
	rtpControlAddr = str2addr(RTPIP, RTPControlPort);
	string tempIP;
	string sdpIPString = "IN IP4 " + RTPIP;
	string sdpPortString = "audio " + to_string(RTPListenPort);
	string listenPortStr = to_string(listenPort);

	string sdpToClientString = "m=audio 5062 RTP/AVP 3\r\na=rtpmap:3 GSM/8000\r\nc=IN IP4 127.0.0.1\r\n";
	//string sdpToClientString = "m=audio 5062 RTP/AVP 118\r\na=sendrecv\r\na=rtpmap:118 AMR-WB/16000/1\r\na=fmtp:118 octet-align=1;mode-change-capability=2;max-red=0\r\na=ptime:20\r\na=maxptime:240\r\nc=IN IP4 127.0.0.1\r\n";
	string sdpToServerString = "m=audio 50010 RTP/AVP 99\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 octet-align=1;mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\nc=IN IP4 127.0.0.1\r\n";
	string qosString = "a=sendrecv\r\na=curr:qos local sendrecv\r\na=curr:qos remote sendrecv\r\na=des:qos mandatory local sendrecv\r\na=des:qos mandatory remote sendrecv\r\n";

    if(bind(socket_fd, (sockaddr*)&listenAddr, sizeof(sockaddr_in)) < 0) {
        cout<<"Error SIP bind"<<endl<<"Exit"<<endl;
        return NULL;
    }
	parser_init();
    while(true) {
        pthread_testcancel();
		addrSize = sizeof(sockaddr);
        ssize_t len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
		cout<<len<<endl;
        if(len >= 0) {

			osip_message_init(&sip);
			err = osip_message_parse(sip, (char*)receiveBuffer, len);
			if(err != 0) {
				cout<<"Error: failed while parsing!"<<endl;
				continue;
			}
			cout<<"parse done"<<endl;

			osip_message_get_via(sip, osip_list_size(&sip->vias)-1, &sip_via);
			osip_free(sip_via->host);
			sip_via->host = (char*)osip_malloc(listenIP.length() + 1);
			strcpy(sip_via->host, listenIP.c_str());
			osip_free(sip_via->port);
			sip_via->port = (char*)osip_malloc(listenPortStr.length() + 1);
			strcpy(sip_via->port, listenPortStr.c_str());
			//remove IMSI and change domain
			if(strlen(sip->from->url->username) > 15) {
				strcpy(sip->from->url->username, sip->from->url->username + 4);
			}
			if(strlen(sip->to->url->username) > 15) {
				strcpy(sip->to->url->username, sip->to->url->username + 4);
			}
			osip_free(sip->from->url->host);
			sip->from->url->host = (char*) osip_malloc(40);
			strcpy(sip->from->url->host, "ims.mcc001.mnc001.3gppnetwork.org");
			osip_free(sip->to->url->host);
			sip->to->url->host = (char*) osip_malloc(40);
			strcpy(sip->to->url->host, "ims.mcc001.mnc001.3gppnetwork.org");
			while(osip_list_size(&sip->headers) > 1) {
				osip_list_remove(&sip->headers, 0);
			}
			osip_header_t* head;
			osip_header_init(&head);
			head->hname = (char*)osip_malloc(10);
			head->hvalue = (char*) osip_malloc(20);
			strcpy(head->hname, "K");
			strcpy(head->hvalue, "100rel");
			osip_list_add(&sip->headers, head, -1);
			osip_header_init(&head);
			head->hname = (char*)osip_malloc(10);
			head->hvalue = (char*) osip_malloc(20);
			strcpy(head->hname, "K");
			strcpy(head->hvalue, "replaces");
			osip_list_add(&sip->headers, head, -1);
			osip_header_init(&head);
			head->hname = (char*)osip_malloc(20);
			head->hvalue = (char*) osip_malloc(20);
			strcpy(head->hname, "Max-Forwards");
			strcpy(head->hvalue, "70");
			osip_list_add(&sip->headers, head, -1);

			if(osip_list_size(&sip->allows) == 0)
				sip->allows = getAllows();
			if(sip->content_type != NULL) {
				cout<<"Body Type: "<<sip->content_type->subtype << endl;
				cout<<string((char*)&receiveBuffer[0])<<endl;
			}
			// change contact
			if(addrCmp(tempAddr, clientAddr) && osip_list_size(&sip->contacts) > 0) {
				contact = (osip_contact_t*)osip_list_get(&sip->contacts, 0);
				if(strlen(contact->url->username) > 15)
					strcpy(contact->url->username, contact->url->username + 4);
				strcpy(contact->url->host, "162.105.85.217");
				contact->displayname = (char*) osip_malloc(100);
				strcpy(contact->displayname, "\"001010123456780@ims.mcc001.mnc001.3gppnetwork.org\"");
			}
			if(sip->content_type != NULL && strcmp("sdp", sip->content_type->subtype)== 0) {
				cout<<"in SDP: "<<to_string(osip_list_size(&sip->bodies))<<endl;
				sdpBody = (osip_body_t*) osip_list_get(&sip->bodies, 0);
				// prepare rtp trans
				string sdp(sdpBody->body);
                err = buildControl(sdp, controlPacket);
				if(err != 0)
					continue;
				controlPacket.callid = sip->call_id->number;
                len = controlPacket.toBuffer((char*)outputBuffer);
                len = sendto(socket_fd, outputBuffer, len, 0, (sockaddr*)&rtpControlAddr, sizeof(sockaddr));
				// change sdp
				if(addrCmp(tempAddr, serverAddr)) {
					sdp = replaceMedia(sdp, sdpToClientString);
				}
				else if(addrCmp(tempAddr, clientAddr)){
					sdp = replaceMedia(sdp, sdpToServerString);
				}
				sdp = replaceSDP(sdp, sdpIPString, sdpPortString);
				cout<<"Replaced SDP: "<<sdp;
				osip_free(sdpBody->body);
				sdpBody->body = (char*)osip_malloc(sdp.length() + 1);
				strcpy(sdpBody->body, sdp.c_str());
				sdpBody->length = sdp.length();
			}
			cout<<"SIP to str"<<endl;
			err = osip_message_to_str(sip, &out_buffer, (size_t*)&len);
			if(addrCmp(tempAddr, serverAddr)) {
				count = (int)sendto(socket_fd, out_buffer, len, 0, (sockaddr*)&clientAddr, sizeof(sockaddr));
			}
			else {
				count = (int)sendto(socket_fd, out_buffer, len, 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
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
