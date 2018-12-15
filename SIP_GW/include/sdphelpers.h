#ifndef SDPHELPERS_H
#define SDPHELPERS_H
#include "rtpcontrolpacket.hpp"
#include "regexsdphelpers.hpp"
#include <string>
using std::string;

static string sdpPortPatten = "audio [0-9]+";
static string numberPatten = "[0-9]+";
static string ipPatten = "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+";
static string AMRPatten = "a=.*AMR-WB.*\r\n";
static string GSMPatten = "a=.*GSM.*\r\n";
static string callidPatten = "Callid";
static string mediaPatten = "m=";
static string activePatten = "t=[0-9]+ [0-9]+";
static string sessIDPatten = "o=.+ [0-9]+ [0-9]+ IN IP4";


static int sdpGetMedia(string& sdp, RTPControlPacket& control) {
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

static string sdpReplaceSDP(const string& src, const string& ipString, const string& portString) {
	string result = replaceAll(ipPatten, src, ipString);
	result = replaceAll(sdpPortPatten, result, portString);
	return result;
}
static string sdpReplaceMedia(const string& src, const string& media) {
	smatch sm;
	if(regex_search(src.cbegin(), src.cend(), sm, regex(mediaPatten, regex_constants::icase)))
		return string(sm.prefix()) + media;
	else
		return src;
}
static string sdpReplaceConnection(const string& src, const string& connection) {
	smatch sm;
	if(regex_search(src.cbegin(), src.cend(), sm, regex(activePatten, regex_constants::icase))) {
		string result = string(sm.prefix()) + connection + activePatten + string(sm.suffix());
		return result;
	}
	else
		return src;
}

#endif
