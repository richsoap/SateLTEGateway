#include <stdio.h>
#include <rtpsession.h>
#include <rtppacket.h>
#include <rtpsessionparams.h>
#include <rtpudpv4transmitter.h>
#include <rtpsourcedata.h>
#include <string.h>
#include <unistd.h>

using namespace jrtplib;

void checkError(int err) {
    if(err < 0) {
        printf("Error:%s\n", RTPGetErrorString(err).c_str());
        exit(-1);
    }
}

int main(int argc, char** argv) {
    RTPSession sess;
    int port;
    int status;
    if(argc != 2) {
        printf("Wrong argv\n");
        return -1;
    }
    port = atoi(argv[1]);

    RTPSessionParams sessionparams;
    RTPUDPv4TransmissionParams transparams;
    //sessionparams.SetUsePollThread(true);
    sessionparams.SetOwnTimestampUnit(1.0/10.0);
    sessionparams.SetAcceptOwnPackets(true);
    transparams.SetPortbase(port);
    status = sess.Create(sessionparams, &transparams);
    checkError(status);

    sess.SetDefaultPayloadType(96);
    sess.SetDefaultMark(false);
    sess.SetDefaultTimestampIncrement(160);
    do {
        if(sess.GotoFirstSourceWithData()) {
            do {
                RTPSourceData* src = sess.GetCurrentSourceInfo();
                RTPPacket* packet;
                while(src->HasData()) {
                    packet = src->GetNextPacket();
                    printf("New packet::\n");
                    for(uint32_t i = 0;i < packet->GetPayloadLength();i ++) {
                        printf("%c", packet->GetPayloadData()[i]);
                    }
                    printf("\n");
                }
            } while(sess.GotoNextSourceWithData());
        }
        usleep(10000);
        sess.Poll();
    } while(1);
    return 0;
}
