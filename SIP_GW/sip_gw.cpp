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
#include "rtpcontrolpacket.hpp"
#include "srsuecontrolpacket.hpp"
#include "imsi.hpp"
#include "siphelpers.hpp"
#include "sdphelpers.hpp"
#include "commonhelpers.hpp"

#define BUFFER_SIZE 10240
#define ADDRSIZE sizeof(sockaddr_in)
#define IMS_DOMAIN "ims.mnc001.mcc001.3gppnetwork.org"

#define DIR_IMS2BTS 0x01
#define DIR_BTS2IMS 0x02


using namespace std;

static int listenPort;
static int controlPort;
static int serverPort;
static int clientPort;
static int RTPControlPort;
static int RTPListenPort;
static string listenIP;
static string clientIP;
static string RTPIP;

static map<IMSI, in_addr> phyaddrMap;
static map<IMSI, in_addr> viraddrMap;

/*
 * Addr helper
 *
 *
 * */

static IMSI getIMSI(const osip_message_t* sip, int dir) {
    IMSI result;
    if(MSG_IS_INVITE(sip) || MSG_IS_ACK(sip) || MSG_IS_REGISTER(sip) || MSG_IS_BYE(sip) || MSG_IS_OPTIONS(sip) ||
            MSG_IS_INFO(sip) || MSG_IS_CANCEL(sip) || MSG_IS_REFER(sip) || MSG_IS_NOTIFY(sip) ||
            MSG_IS_SUBSCRIBE(sip) || MSG_IS_PRACK(sip) || MSG_IS_UPDATE(sip) || MSG_IS_PUBLISH(sip)) {
		if(dir == DIR_BTS2IMS) {
			if(sip->from->url != NULL)
				memcpy(&result, sip->from->url->username, sizeof(result));
			else
			    memset(&result, 0, sizeof(result));
		}
		else{
			if(sip->to->url != NULL)
				memcpy(&result, sip->to->url->username, sizeof(result));
			else
				memset(&result, 0, sizeof(result));
		}
    }
    else {
		if(dir == DIR_BTS2IMS) {
			if(sip->to->url != NULL)
				memcpy(&result, sip->to->url->username, sizeof(result));
			else
			    memset(&result, 0, sizeof(result));
		}
		else {
			if(sip->to->url != NULL)
				memcpy(&result, sip->to->url->username, sizeof(result));
			else
				memset(&result, 0, sizeof(result));
		}
    }
    return result;
}

/*
 *
 * Threads
 *
 * */

static void* SIPControlThread(void* input) {
   int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
   if(socket_fd < 0) {
    cout<<"Error SIP Control Socket"<<endl;
    return NULL;
   }
   sockaddr_in controlAddr;
   srsueControlPacket packet;
   in_addr addr;
   uint8_t receiveBuffer[BUFFER_SIZE];
   controlAddr = str2addr(listenIP, controlPort);
   if(bind(socket_fd, (sockaddr*) &controlAddr, sizeof(sockaddr_in)) < 0) {
       cout<<"Error SIP Control Bind"<<endl;
       return NULL;
   }
   while(true) {
       pthread_testcancel();
       ssize_t len = recv(socket_fd, receiveBuffer, BUFFER_SIZE, 0);
       srsueControlPacketParse(packet, receiveBuffer);
       switch(packet.event) {
           case SRSUE_ADD_PHYADDR:
               memcpy(&addr, &packet.data[0], sizeof(addr));
               phyaddrMap[packet.imsi] = addr;
			   cout<<"Add addr:";
			   printAddrIn(addr);
               break;
           case SRSUE_ADD_VIRADDR:
               memcpy(&addr, &packet.data[0], sizeof(addr));
               viraddrMap[packet.imsi] = addr;
               break;
           default:
               continue;
       }
   }
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
    sockaddr_in clientAddr;
	sockaddr_in rtpControlAddr;
	socklen_t addrSize;
    uint8_t receiveBuffer[BUFFER_SIZE];
    uint8_t tempBuffer[BUFFER_SIZE];
	int err;
	int count;
    RTPControlPacket controlPacket;
	osip_message_t* sip;
	char* out_buffer;
	char* temp_point;

	listenAddr = str2addr(listenIP, listenPort);
	serverAddr = str2addr(listenIP, listenPort);
	clientAddr = str2addr(clientIP, clientPort);
	rtpControlAddr = str2addr(RTPIP, RTPControlPort);
	string tempIP;
	string sdpIPString = RTPIP;
	string sdpPortString = "audio " + to_string(RTPListenPort);
	string listenPortStr = to_string(listenPort);
	string sdpToClientString = "m=audio 5062 RTP/AVP 3\r\na=rtpmap:3 GSM/8000/1\r\n";
	string sdpToIMSString = "m=audio 50010 RTP/AVP 99\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 octet-align=1;mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\n";
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
            if(addrCmp(tempAddr, clientAddr)) {
				IMSI imsi = getIMSI(sip, DIR_BTS2IMS);
				if(viraddrMap.count(imsi) == 0)
					continue;
				string virIP = string(inet_ntoa(viraddrMap[imsi]));
                sipSetVia(sip,virIP , listenPort); 
                sipSetDomains(sip, IMS_DOMAIN);
                sipRemoveIMSI(sip);
                sipAddHeaders(sip);
                if(osip_list_size(&sip->allows) == 0)
					sip->allows = sipGenerateAllows();
                sipSetContact(sip, virIP, listenPort);
				sipRemoveRoute(sip);
                string sdp = sipGetSDP(sip);
                if(sdp.length() > 0) {
                    if(sdpGetMedia(sdp, controlPacket) != 0)
                        continue;
                    controlPacket.callid = sip->call_id->number;
                    controlPacket.payload = 3;
                    controlPacket.code = CONTROL_GSM;
                    
                    sdp = sdpReplaceMedia(sdp, sdpToIMSString);
				    sdp = sdpReplaceConnection(sdp, "c=IN IP4 127.0.0.1\r\n");
                    sdp = sdpReplaceSDP(sdp, virIP, sdpPortString); 
                    sipSetSDP(sip, sdp);
                    if(MSG_IS_INVITE(sip))
                        controlPacket.slot = 0;
                    else
                        controlPacket.slot = 3;
            		controlPacket.print();
			    	len = controlPacket.toBuffer((char*)tempBuffer);
                    len = sendto(socket_fd, tempBuffer, len, 0, (sockaddr*)&rtpControlAddr, sizeof(sockaddr));
                }

            }
            else {
                sipSetVia(sip, listenIP, listenPort);
                string sdp = sipGetSDP(sip);
                if(sdp.length() > 0) {
                    if(sdpGetMedia(sdp, controlPacket) != 0)
                        continue;
                    controlPacket.callid = sip->call_id->number;
                    controlPacket.payload = 99;
                    controlPacket.code = CONTROL_AMR;

                    sdp = sdpReplaceMedia(sdp, sdpToClientString);
                    sdp = sdpReplaceConnection(sdp, "c=IN IP4 127.0.0.1\r\n");
                    sdp = sdpReplaceSDP(sdp, sdpIPString, sdpPortString);
                    sipSetSDP(sip, sdp);
                    if(MSG_IS_INVITE(sip))
                        controlPacket.slot = 2;
                    else
                        controlPacket.slot = 1;
                    len = controlPacket.toBuffer((char*)tempBuffer);
                    len = sendto(socket_fd, tempBuffer, len, 0, (sockaddr*)&rtpControlAddr, sizeof(sockaddr));
                }
            }
            err = osip_message_to_str(sip, &out_buffer, (size_t*)&len);
            if(err != 0) {
                continue;
            }
            if(addrCmp(tempAddr, clientAddr)) {
                IMSI imsi = getIMSI(sip, DIR_BTS2IMS);
                if(phyaddrMap.count(imsi)) {
                    serverAddr.sin_addr = phyaddrMap[imsi];
                    count = (int)sendto(socket_fd, out_buffer, len, 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
                    osip_free(out_buffer);
                }
            } 
            else {
				count = (int)sendto(socket_fd, out_buffer, len, 0, (sockaddr*)&clientAddr, sizeof(sockaddr));
                osip_free(out_buffer);
            }
        }
    }
    return NULL;
}

int main(int argc, char** argv) {
    if(argc != 8) {
        cout<<"Wrong argv: ListenIP ListenPort ClientIP ClientPort RTPIP RTPControlPort RTPlistenPort"<<endl;
        exit(-1);
    }
    listenIP = argv[1];
    listenPort = atoi(argv[2]);
	clientIP = argv[3];
	clientPort = atoi(argv[4]);
	RTPIP = argv[5];
	RTPControlPort = atoi(argv[6]);
	RTPListenPort = atoi(argv[7]);
    string buffer;
    pthread_t SIPTid;
    pthread_t SIPControlTid;
    pthread_create(&SIPTid, NULL, &SIPThread, NULL);
    pthread_create(&SIPControlTid, NULL, &SIPControlThread, NULL);
    while(cin>>buffer) {
        if(buffer.compare("quit") == 0) {
            break;
        }
    }
    return 0;
}
