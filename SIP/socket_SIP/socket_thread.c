#include <pj/log.h>
#include <pj/pool.h>
#include <pjmedia/port.h>
#include "socket_send.c"
#include "socket_rece.c"

#define THIS_FILE "socket_thread.cpp"

struct socket_thread_conf {
    socket_send_port *send_port;
    char* buffer;
    int sock_fd;
};

static socket_thread_conf *init_thread_conf(socket_send_port* _send_port,
                                        const char* udp_ip,
                                        int udp_port) {
    socket_thread_conf *conf = PJ_POOL_ZALLOC_T(_send_port->pool, struct socket_thread_conf);
    if(!conf) {
        pjsua_perror(THIS_FILE, "Unable to alloc mem from pool", PJ_ENOMEM);
        pj_log_pop_indent();
        return NULL;
    }
    conf->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(conf->sock_fd < 0) {
        pjsua_perror(THIS_FILE, "Unable to get socket fd", PJ_ENOMEM);
        pj_log_pop_indent();
        return NULL;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, udp_ip, addr.sin_addr);
    addr.sinport = htons(udp_port);
    conf->send_port = _send_port;
    conf->buffer = pj_pool_calloc(_send_port->pool, _send_port->buffer_size, char);
    if(bind(conf->sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        pjsua_perror(THIS_FILE, "Unable to bind port", PJ_ENOMEM);
        pj_log_pop_indent();
        return NULL;
    }
    return conf;
}

static void socket_thread() {

}

