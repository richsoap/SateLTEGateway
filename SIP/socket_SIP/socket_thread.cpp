#include <pj/log.h>
#include <pj/pool.h>
#include <pj/types.h>
#include <pjlib.h>
#include <pjsua.h>
#include <pjmedia/port.h>
#include "socket_port.h"
#include "socket_thread.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define THIS_FILE "socket_thread.cpp"
#define THREAD_BUFFER_SIZE 10240

namespace pj{
int socket_thread::socket_fd = -1;
int socket_thread::socket_record_fd = -1;
volatile int socket_thread::thread_quit_flag = 0;
pjmedia_port* socket_thread::thread_play_port = NULL;
pjmedia_port* socket_thread::thread_record_port = NULL;
sockaddr_in* socket_thread::thread_record_addr = NULL;
sockaddr_in* socket_thread::thread_play_addr = NULL;
pj_pool_t* socket_thread::thread_pool = NULL;
pj_thread_t *socket_thread::thread_play;

void socket_thread::thread_pool_init() {
    thread_pool = thread_pool == NULL? pjsua_pool_create("Thread pool", 1000, 1000): thread_pool;
}

void* socket_thread::thread_play_thread(void* input) {
    pj_thread_desc desc;
    pj_thread_t *this_thread;
    pj_status_t status;

    pj_bzero(desc, sizeof(desc));

    char* buffer = (char*) pj_pool_alloc(thread_pool, THREAD_BUFFER_SIZE);
    status = pj_thread_register("play thread", desc, &this_thread);
    int timeout = 1000;
    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(int));
    for(;thread_quit_flag == 0;) {
        ssize_t len = read(socket_fd, buffer, THREAD_BUFFER_SIZE);
        if(len > 0 && thread_play_port != NULL) {
            socket_send_port* sock_port = (socket_send_port*) thread_play_port;
            pj_memcpy(sock_port->buffer, buffer, sock_port->data_len);
        }
    }
    return NULL;
}

int socket_thread::create_play_thread(const char *listen_ip, int listen_port) {
    pj_status_t status;
    thread_pool_init();
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_fd < 0) {
        pjsua_perror(THIS_FILE, "Unable to get socket fd", PJ_ENOMEM);
        pj_log_pop_indent();
        return -1;
    }

    if(thread_play_addr == NULL)
        thread_play_addr = PJ_POOL_ZALLOC_T(thread_pool, sockaddr_in);
    memset(thread_play_addr, 0, sizeof(sockaddr_in));
    thread_play_addr->sin_family = AF_INET;
    inet_pton(AF_INET, listen_ip, &thread_play_addr->sin_addr);
    thread_play_addr->sin_port = htons(listen_port);
    if(bind(socket_fd, (sockaddr*) thread_play_addr, sizeof(sockaddr_in)) < 0) {
        pjsua_perror(THIS_FILE, "Unable to bind port", PJ_ENOMEM);
        pj_log_pop_indent();
        return -1;
    }
    thread_quit_flag = 0;
    status = pj_thread_create(thread_pool, "play_thread", (pj_thread_proc*)&thread_play_thread,
                    NULL, PJ_THREAD_DEFAULT_STACK_SIZE,
                    0, &thread_play);
    if(status != PJ_SUCCESS) {
        PJ_LOG(3, (THIS_FILE, "create thread error"));
        return -1;
    }
    return 0;
}

void socket_thread::exit_play_thread() {
    thread_quit_flag = 1;
    pj_thread_join(thread_play);
    pj_pool_release(thread_pool);
}

void socket_thread::thread_add_record_port(pjmedia_port* port, const char* udp_ip, int udp_port) {
    thread_pool_init();
    thread_record_port = port;
    if(thread_record_addr == NULL)
        thread_record_addr = PJ_POOL_ZALLOC_T(thread_pool, sockaddr_in);
    memset(thread_record_addr, 0, sizeof(sockaddr_in));
    thread_record_addr->sin_family = AF_INET;
    inet_pton(AF_INET, udp_ip, &thread_record_addr->sin_addr);
    thread_record_addr->sin_port = htons(udp_port);
    if(socket_record_fd < 0)
        socket_record_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

void socket_thread::thread_add_play_port(pjmedia_port* port) {
    thread_play_port = port;
}

void socket_thread::thread_remove_port(pjmedia_port* port) {
    // TODO map<pjmedia_port, info>
}

void socket_thread::thread_record_frame(pjmedia_port* port, pjmedia_frame* frame) {
    if(port == thread_record_port) {
        sendto(socket_fd, frame->buf, frame->size, 0, (sockaddr*) thread_record_addr, sizeof(sockaddr));
    }
}
}
