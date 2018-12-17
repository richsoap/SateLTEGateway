#ifndef COMMONHELPERS_H
#define COMMONHELPERS_H
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using std::cout;
using std::endl;

static void printAddr(const sockaddr_in& addr) {
	char buffer_test[20];
	inet_ntop(AF_INET, &addr.sin_addr, buffer_test, 20);
	cout<<buffer_test<<":"<<ntohs(addr.sin_port)<<endl;
}
static void printAddrIn(const in_addr& addr) {
	char buffer_test[20];
	inet_ntop(AF_INET, &addr, buffer_test, 20);
	cout<<buffer_test;
}

static bool addrCmp(const sockaddr_in& a1, const sockaddr_in& a2) {
	return memcmp(&a1.sin_addr, &a2.sin_addr, sizeof(a1.sin_addr)) == 0 && a1.sin_port == a2.sin_port;
}

static sockaddr_in str2addr(string IP, int port) {
	sockaddr_in result;
	result.sin_family = AF_INET;
	result.sin_port = htons(port);
	inet_pton(AF_INET, IP.c_str(), &result.sin_addr);
	return result;
}


#endif
