#include <stdio.h>
#include <string.h>
#include <rtpsession.h>
#include <rtperrors.h>
#include <rtpipv4address.h>
#include <rtpsessionparams.h>
#include <rtpudpv4transmitter.h>


using namespace jrtplib;
void checkError(int err) {
    if(err < 0) {
        printf("Error:%s\n", RTPGetErrorString(err).c_str());
        exit(-1);
    }
}

int main(int argc, char** argv) {
    RTPSession sess;
    uint8_t ip[] = {127,0,0,1};
    int dstport;
    int srcport = 6000;
    int status, index;
    char buffer[128];

    if(argc != 3) {
        printf("Wrong Argv\n");
        return -1;
    }
    srcport = atoi(argv[1]);
    dstport = atoi(argv[2]);
    RTPIPv4Address addr(ip, dstport);

    RTPSessionParams sessionparams;
    RTPUDPv4TransmissionParams transparams;
    //sessionparams.SetUsePollThread(true);
    sessionparams.SetOwnTimestampUnit(1.0/10.0);
    sessionparams.SetAcceptOwnPackets(true);
    transparams.SetPortbase(srcport);

    sess.Create(sessionparams, &transparams);
    sess.SetDefaultPayloadType(96);
    sess.SetDefaultMark(false);
    //sess.SetDefaultTimeStampIncrement(10);

    status = sess.AddDestination(addr);
    index = 1;
    do {
        scanf("%s", buffer);
        if(buffer[0] == ']')
            break;
        //sess.SendPacket(buffer, strlen(buffer));
        sess.SendPacket(buffer, strlen(buffer), 96, false, 10);
        printf("Send RTP packet: %d %d\n", index ++, strlen(buffer));
    } while(true);
    return 0;
}
