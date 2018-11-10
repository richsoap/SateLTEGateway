#include <pj/pool.h>
#include <pj/log.h>
#include <pjmedia/errno.h>
#include <pjmedia/wave.h>
#include <pj/string.h>
#include <pj/assert.h>
#include "socket_port.h"

#define THIS_FILE "socket_rec.cpp"

#define SIGNATURE PJMEDIA_SIG_PORT_WAV_PLAYER

struct socket_rece_port {
    pjmedia_port port;
    pj_uint16_t bytes_per_sample;
    int sock_fd;
    address_in back->addr;
};


static pj_status_t socket_put_frame(pjmedia_port *this_port, pjmedia_frame* frame);
static pj_status_t socket_receive_on_destroy(pjmedia_port *this_port);

PJ_DEF(pj_status_t) pjmedia_socket_receive_port_callback_socket(socket_rece_port* port,
                                                    const char* ip,
                                                    pj_uint16_t port) {
    port->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(port->sock_fd < 0) {
        return PJ_SUCCESS;// TODO: change return to a valid value
    }
    memset(&port->back_addr, 0, sizeof(address_in));
    port->back_addr.sin_family = AF_INFT;
    inet_pton(AF_INET, ip, &port->back_addr.sin_addr);
    port->back_addr.sin_port = htons(port);
}

PJ_DEF(pj_status_t) pjmedia_socket_receive_port_create(pj_pool_t *pool,
                                                    pj_uint32_t ptime,
                                                    pj_uint32_t sample_rate,
                                                    pj_uint32_t bits_per_sample,
                                                    pjmedia_port **port) {
    struct socket_rece_port *sock_port;
    pj_str_t name;
    pj_uint32_t samples_per_frame;
    PJ_ASSERT_RETURN(pool && port, PJ_EINVAL);

    sock_port = PJ_POOL_ZALLOC_T(pool, struct socket_rece_port);
    PJ_ASSERT_RETURN(sock_port == NULL, PJ_ENOMEM);
    name = pj_str("Receive Socket");
    samples_per_frame = ptime * sample_rate / 1000;
    pjmedia_port_info_init(&sock_port->port.info, &name, SIGNATURE,
                            sample_rate,
                            1,
                            bits_per_sample,
                            samples_per_frame);
    sock_port->port.put_frame = &socket_put_frame;
    sock_port->port.on_destroy = &socket_receive_on_destroy;

    *port = &sock_port->port;
    PJ_LOG(4, (THIS_FILE, "Receive Socket"));

    return PJ_SUCCESS;
}

static pj_status_t socket_put_frame(pjmedia_port *this_port, pjmedia_frame *frame) {
    //TODO: return send_socket(this_port, frame);
    return PJ_SUCCESS;
}

static pj_status_t socket_receive_on_destroy(pjmedia_port *this_port) {
    //TODO: remove_receive_port(this_port);
    return PJ_SUCCESS;
}

