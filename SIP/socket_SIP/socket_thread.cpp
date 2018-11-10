#include <pj/log.h>
#include <pj/pool.h>
#include <pjmedia/port.h>
#include "socket_send.c"
#include "socket_rece.c"

#define THIS_FILE "socket_thread"

struct socket_thread_conf {
    socket_send_port *send_port;
    char* buffer;
    int sock_fd;
    sockaddr_in callback_addr;
};

static socket_thread_conf *init_thread_conf(socket_send_port* _send_port,
                                        const char* udp_ip,
                                        int udp_port) {
    socket_thread_conf *conf = PJ_POOL_ZALLOC_T(_send_port->pool, struct socket_thread_conf);
    if(!conf)
        return NULL;
    conf->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(conf->sock_fd < 0) {
        // TODO: PJLOG
        return NULL;
    }
    memset(&conf->callback_addr, 0, sizeof(sockaddr_in));
    conf->callback_addr.sin_family = AF_INET;
    inet_pton(AF_INET, udp_ip, &conf->callback_addr.sin_addr);
    conf->callback_addr.sinport = htons(udp_port);
    conf->send_port = _send_port;
    conf->buffer = pj_pool_calloc(_send_port->pool, _send_port->buffer_size, char)
   // TODO bind 
}

static void socket_thread() {

}

