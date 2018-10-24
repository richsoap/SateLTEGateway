#include "client.h"
#include "basicIE.h"
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <errno.h>
using namespace std;

SctpClient::SctpClient():echoToAll(0) {}
SctpClient::~SctpClient() { close(sendSocket); }

void SctpClient::start(void) { makeSocket(); }

void SctpClient::makeSocket(void) {
	struct sctp_sndrcvinfo info;
	int result;
	sendSocket = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
	if(sendSocket < 0) {
		printf("SCTP socket wrong\n");
		exit(1);
	}
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(36412);
	inet_pton(AF_INET, "10.128.171.12", &serverAddr.sin_addr);
    //inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	bzero(&events, sizeof(events));
	events.sctp_data_io_event = 1;
	result = setsockopt(sendSocket, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events));
	printf("set result: %d\n", result);
	
	sctp_sendFile(stdin, sendSocket, (struct sockaddr* )&serverAddr, sizeof(serverAddr));
}


void sctp_sendFile(FILE *fp, int sock_fd, struct sockaddr *target, socklen_t tolen) {
	struct sockaddr_in peeraddr;
	struct sctp_sndrcvinfo info;
	struct sockaddr_un mini_addr;
	int mini_fd, maxfd;
	fd_set read_fps, read_fps_cpy;
	sctp_assoc_t id;
	char sendline[MAXLINE];
	char recvline[MAXLINE];
	socklen_t len;
	int out_sz,rd_sz;
	int msg_flags;
	int result;
	char testS1ap[] = 
	{0x00,0x11,0x00,0x2d,0x00,0x00,0x04,0x00,0x3b,0x00,0x08,0x00,0x00,0xf1,0x10,0x00,
	0x1a,0x2d,0x00,0x00,0x3c,0x40,0x0a,0x03,0x80,0x65,0x6e,0x62,0x31,0x61,0x32,0x64,
	0x30,0x00,0x40,0x00,0x07,0x00,0x00,0x00,0x40,0x00,0xf1,0x10,0x00,0x89,0x40,0x01,0x40};
    printf("signal size: %d\n", (int)sizeof(testS1ap));
	bzero(&info, sizeof(info));
	result = sctp_connectx(sock_fd, target, 1, &id);
	if(result != 0) {
		printf("connect error\nerrno is %d\n", errno);
		exit(2);
	}
	info.sinfo_stream = 0;
	info.sinfo_ppid = 0x12;
	info.sinfo_assoc_id = id;

	while(true) {
		fgets(sendline, MAXLINE, fp);
		if(sendline[0] != '[') {
			printf("ERROR\n");
			continue;
		}
        // TODO: size
        rd_sz = (int)sizeof(testS1ap);
		int count = sctp_sendmsg(sock_fd, testS1ap, rd_sz, (struct sockaddr*) target, tolen, 0x00000012, 0, 0, 0, 0);
		printf("need to send %d\nsend count is %d\n", (int)rd_sz, count);
	    rd_sz = sctp_recvmsg(sock_fd, recvline, MAXLINE, (struct sockaddr*) &peeraddr, &len, &info, &msg_flags);
        printf("recv count is %d\n", rd_sz);
	}
}
