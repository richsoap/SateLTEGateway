#include <pthread.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <string>
#include <regex>
#include <map>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "rtpcontrolpacket.hpp"
#include "srsuecontrolpacket.hpp"
#include "imsi.hpp"
#include "siphelpers.hpp"
#include "sdphelpers.hpp"
#include "commonhelpers.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#define BUFFER_SIZE 10240
#define ADDRSIZE sizeof(sockaddr_in)
#define IMS_DOMAIN "ims.mnc001.mcc001.3gppnetwork.org"
#define OPENBTS_DOMAIN "openbts.org"

#define DIR_IMS2BTS 0x01
#define DIR_BTS2IMS 0x02


using namespace std;
namespace bpo = boost::program_options;

static int listenPort;
static int controlPort;
static int serverPort;
static int clientPort;
static int RTPControlPort;
static int RTPListenPort;
static string listenIP;
static string clientIP;
static string RTPIP;

static string config_file;

static map<IMSI, in_addr> phyaddrMap;
static map<IMSI, in_addr> viraddrMap;
static map<string, IMSI> telIMSIMap;
static map<IMSI, string> sipBufferMap;

/*
 *
 * Program arguments processing
 *
 */

static void startParse(int argc, char* argv[]) {
	bpo::options_description common("Configuration options");
	common.add_options()
		("sip.ip", bpo::value<string>(&listenIP)->default_value("0.0.0.0"), "Local host IP")
		("sip.data_port", bpo::value<int>(&listenPort)->default_value(5060), "Local SIP Port")
		("sip.control_port", bpo::value<int>(&controlPort)->default_value(5080), "Local SIP Control Port")
		("bts.ip", bpo::value<string>(&clientIP)->default_value("0.0.0.0"), "OpenBTS IP")
		("bts.port", bpo::value<int>(&clientPort)->default_value(5060), "OpenBTS Port")
		("rtp.ip", bpo::value<string>(&RTPIP)->default_value("0.0.0.0"), "RTP Gateway IP")
		("rtp.data_port", bpo::value<int>(&RTPListenPort)->default_value(10020), "RTP Data Trans Port")
		("rtp.control_port", bpo::value<int>(&RTPControlPort)->default_value(5062), "RTP Control Port")
		("srsue.sip_port", bpo::value<int>(&serverPort)->default_value(5060), "srsUE SIP Port");
// config file
	bpo::options_description position("Config file options");
	position.add_options()
		("config_file", bpo::value<string>(&config_file)->default_value("sip.conf"), "SIP Gateway config file location");
	bpo::positional_options_description p;
	p.add("config-file", -1);

	bpo::options_description cmdOptions;
	cmdOptions.add(common).add(position);

	bpo::variables_map varMap;
	bpo::store(bpo::command_line_parser(argc, argv).options(cmdOptions).positional(p).run(), varMap);
	bpo::notify(varMap);

	cout<<"Reading config file "<< config_file<<" ..."<<endl;
	ifstream confReader(config_file.c_str(), ios::in);
	if(confReader.fail()) {
		cout<<"Failed to read";
		exit(-1);
	}
	bpo::store(bpo::parse_config_file(confReader, common), varMap);
	bpo::notify(varMap);
    cout<<"sip.ip="<<listenIP<<endl;
    cout<<"sip.data_port="<<listenPort<<endl;
    cout<<"sip.control_port="<<controlPort<<endl;
    cout<<"bts.ip="<<clientIP<<endl;
    cout<<"bts.port="<<clientPort<<endl;
    cout<<"rtp.ip="<<RTPIP<<endl;
    cout<<"rtp.data_port="<<RTPListenPort<<endl;
    cout<<"rtp.control_port="<<RTPControlPort<<endl;
    cout<<"srsue.port="<<serverPort<<endl;

}



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
			if(sip->from->url != NULL) {
				if(sip->from->url->scheme[0] != 's' && sip->from->url->scheme[0] != 'S') {
					string tel = string(sip->from->url->username);
					if(telIMSIMap.count(tel) != 0) 
						result = telIMSIMap[tel];
					else
						memset(&result, 0, sizeof(result));
				}
				else {
					if(sip->from->url->username[0] != 'I')
						memcpy(&result, sip->from->url->username, sizeof(result));
					else
						memcpy(&result, sip->from->url->username + 4, sizeof(result));
				}
			}
			else
			    memset(&result, 0, sizeof(result));
		}
		else{
			if(sip->to->url != NULL) {
				if(sip->to->url->scheme[0] != 's' && sip->to->url->scheme[0] != 'S') {
					string tel = string(sip->to->url->username);
					if(telIMSIMap.count(tel) != 0)
						result = telIMSIMap[tel];
					else
						memset(&result, 0, sizeof(result));
				}
				else {
					if(sip->to->url->username[0] != 'I')
						memcpy(&result, sip->to->url->username, sizeof(result));
					else
						memcpy(&result, sip->to->url->username + 4, sizeof(result));
				}
			}
			else
				memset(&result, 0, sizeof(result));
		}
    }
    else {
		if(dir == DIR_BTS2IMS) {
			if(sip->to->url != NULL) {
				if(sip->to->url->username[0] != 'I')
					memcpy(&result, sip->to->url->username, sizeof(result));
				else
					memcpy(&result, sip->to->url->username + 4, sizeof(result));
			}
			else
			    memset(&result, 0, sizeof(result));
		}
		else {
			if(sip->from->url != NULL)
				memcpy(&result, sip->from->url->username, sizeof(result));
			else
				memset(&result, 0, sizeof(result));
		}
    }
    return result;
}

static void printIMSI(const IMSI& imsi) {
	for(int i = 0;i < 15;i ++)
		cout << imsi.val[i];
	cout<<endl;
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
   sockaddr_in tempAddr;
   socklen_t addrSize;
   srsueControlPacket packet;
   in_addr addr;
   uint8_t receiveBuffer[BUFFER_SIZE];
   controlAddr = str2addr(listenIP, controlPort);
   if(bind(socket_fd, (sockaddr*) &controlAddr, sizeof(sockaddr_in)) < 0) {
       cout<<"Error SIP Control Bind"<<endl;
       return NULL;
   }
   addrSize = sizeof(sockaddr_in);
   while(true) {
       pthread_testcancel();
       ssize_t len = recvfrom(socket_fd, receiveBuffer, BUFFER_SIZE, 0, (sockaddr*)&tempAddr, &addrSize);
       srsueControlPacketParse(packet, receiveBuffer);
       switch(packet.event) {
           case SRSUE_ADD_VIRADDR:
			   cout<<"Control: ";
			   printIMSI(packet.imsi);
               memcpy(&addr, packet.data, sizeof(addr));
               viraddrMap[packet.imsi] = addr;
			   phyaddrMap[packet.imsi] = tempAddr.sin_addr;
			   cout<<"Add addr: ";
			   printAddrIn(addr);
			   cout<<endl;
               break;
           default:
               continue;
       }
   }
}

static void* SIPThread(void* input) {
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
	telIMSIMap["18292176260"] = "001010123456789";

	listenAddr = str2addr(listenIP, listenPort);
	serverAddr = str2addr(listenIP, serverPort);
	clientAddr = str2addr(clientIP, clientPort);
	rtpControlAddr = str2addr(RTPIP, RTPControlPort);
	string tempIP;
	string sdpIPString = RTPIP;
	string sdpPortString = "audio " + to_string(RTPListenPort);
	string listenPortStr = to_string(listenPort);
	string sdpToClientString = "m=audio 5062 RTP/AVP 3\r\na=rtpmap:3 GSM/8000/1\r\n";
#ifdef NOALIGN
	string sdpToIMSString = "m=audio 50010 RTP/AVP 99\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\n";
#else
	string sdpToIMSString = "m=audio 50010 RTP/AVP 99\r\na=rtpmap:99 AMR-WB/16000/1\r\na=fmtp:99 octet-align=1;mode-change-capability=2;max-red=0\r\na=maxptime:240\r\na=ptime:20\r\n";
#endif
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
			if(MSG_IS_BYE(sip)) {
				controlPacket.command = CONTROL_REMOVE;
				controlPacket.callid = sip->call_id->number;
				len = controlPacket.toBuffer((char*)tempBuffer);
                len = sendto(socket_fd, tempBuffer, len, 0, (sockaddr*)&rtpControlAddr, sizeof(sockaddr));
			}

			if(MSG_IS_REGISTER(sip)) {
			
				
			}

            if(addrCmp(tempAddr, clientAddr)) {
				IMSI imsi = getIMSI(sip, DIR_BTS2IMS);
				cout<<"Receive SIP from client :";
				printIMSI(imsi);
				if(viraddrMap.count(imsi) == 0)
					continue;
				string virIP = string(inet_ntoa(viraddrMap[imsi]));
				sipSetVia(sip,"TCP", virIP, listenPort);
				cout<<"Set vias\n";
                sipRemoveIMSI(sip);
				cout<<"Remove IMSI\n";
				sipSetDomains(sip, IMS_DOMAIN);
				cout<<"Set Domains\n";
                if(osip_list_size(&sip->allows) == 0)
					sip->allows = sipGenerateAllows();
                sipSetContact(sip, virIP, listenPort);
				cout<<"Set contact\n";
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
                sipSetVia(sip,"UDP", listenIP, listenPort);
				if(sipIsTel(sip->to->url)) {
					string tel(sip->to->url->string);
					if(telIMSIMap.count(tel) != 0) {
						IMSI imsi = telIMSIMap[tel];
						printIMSI(imsi);
						sipSetUri(sip->to->url, &imsi);
					}
					else {
						sipSetUri(sip->to->url, NULL);
					}
					deleteCharValue(&sip->to->url->string);
				}
				if(sipIsTel(sip->from->url)){
					string tel(sip->from->url->string);
					if(telIMSIMap.count(tel) != 0) {
						IMSI imsi = telIMSIMap[tel];
						sipSetUri(sip->from->url, &imsi);
					}
					else {
						sipSetUri(sip->from->url, NULL);
					}
					deleteCharValue(&sip->from->url->string);
				}
				cout<<"Tel to sip\n";
				sipSetDomains(sip, OPENBTS_DOMAIN);
				cout<<"Set Domains\n";
                string sdp = sipGetSDP(sip);
                if(sdp.length() > 0) {
                    if(sdpGetMedia(sdp, controlPacket) != 0)
                        continue;
                    controlPacket.callid = sip->call_id->number;
                    controlPacket.payload = 99;
                    controlPacket.code = CONTROL_AMR;
					IMSI imsi = getIMSI(sip, DIR_IMS2BTS);
					controlPacket.addr.sin_addr = phyaddrMap[imsi];

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
					if(MSG_IS_REGISTER(sip)) {
						sipBufferMap[imsi] = string(out_buffer, len);
					}
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


static void* SIPKeepAlive(void* input) {
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_fd < 0) {
		cout<<"Error SIP Keep Alice Socket"<<endl;
		return NULL;
	}
	sockaddr_in serverAddr = str2addr(listenIP, serverPort);
	while(true) {
		sleep(1800);
		int send_num = 0;
		for(auto it:sipBufferMap) {
			if(phyaddrMap.count(it.first)) {
				serverAddr.sin_addr = phyaddrMap[it.first];
				int count = (int)sendto(socket_fd, it.second.c_str(), it.second.length(), 0, (sockaddr*)&serverAddr, sizeof(sockaddr));
				if(send_num > 0)
					send_num ++;
			}	
		}
		cout<<"Keep Alive---"<<send_num<<endl;
	}
	return NULL;

}


int main(int argc, char** argv) {
	startParse(argc, argv);
    string buffer;
    pthread_t SIPTid;
    pthread_t SIPControlTid;
	pthread_t SIPKeepAliveTid;
    pthread_create(&SIPTid, NULL, &SIPThread, NULL);
    pthread_create(&SIPControlTid, NULL, &SIPControlThread, NULL);
	pthread_create(&SIPKeepAliveTid, NULL, &SIPKeepAlive, NULL);
    while(cin>>buffer) {
        if(buffer.compare("quit") == 0) {
            break;
        }
    }
    return 0;
}
