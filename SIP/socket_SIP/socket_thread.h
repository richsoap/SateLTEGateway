#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H
#include <pjmedia/port.h>
#include "socket_send.c"
#include "socket_rece.c"

static void* create_play_thread(const char* udp_ip, int udp_port);
static void exit_play_thread();
static void add_port(pjmedia_port* port, const char* udp_ip, int udp_port);
static void remove_port(pjmedia_port* port);
static void record_frame(pjmedia_port* port, pjmedia_frame* frame);

#endif
