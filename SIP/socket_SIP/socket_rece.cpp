#include <pj/pool.h>
#include <pj/log.h>
#include <pjmedia/errno.h>
#include <pjmedia/wave.h>
#include <pj/string.h>
#include <pj/assert.h>
#include "socket_port.h"
#include "socket_thread.h"
#define THIS_FILE "socket_rec.cpp"

#define SIGNATURE PJMEDIA_SIG_PORT_WAV_PLAYER

static pj_status_t socket_put_frame(pjmedia_port *this_port, pjmedia_frame *frame);
static pj_status_t socket_receive_on_destroy(pjmedia_port *this_port);


/*struct socket_rece_port {
    pjmedia_port port;
    pj_uint16_t bytes_per_sample;
};*/


PJ_DEF(pj_status_t) pjmedia_socket_receive_port_create(pj_pool_t *pool,
                                                    pj_uint32_t ptime,
                                                    pj_uint32_t sample_rate,
                                                    pj_uint32_t bits_per_sample,
                                                    pjmedia_port **port) {
    socket_rece_port *sock_port = NULL;
    pj_str_t name;
    pj_uint32_t samples_per_frame;
    PJ_ASSERT_RETURN(pool && port, PJ_EINVAL);

    sock_port = PJ_POOL_ZALLOC_T(pool, socket_rece_port);
    name = pj_str("Receive Socket");
    samples_per_frame = ptime * sample_rate * bits_per_sample / 1000 / 8;
    pjmedia_port_info_init(&sock_port->port.info, &name, SIGNATURE,
                            sample_rate,
                            1,
                            bits_per_sample,
                            samples_per_frame);
    PJ_LOG(4, (THIS_FILE, "Handful checkpoint"));
    sock_port->port.put_frame = &socket_put_frame;
    sock_port->port.on_destroy = &socket_receive_on_destroy;

    *port = &sock_port->port;
    PJ_LOG(4, (THIS_FILE, "Receive Socket"));

    return PJ_SUCCESS;
}

static pj_status_t socket_put_frame(pjmedia_port *this_port, pjmedia_frame *frame) {
    pj::socket_thread::thread_record_frame(this_port, frame);
    return PJ_SUCCESS;
}

static pj_status_t socket_receive_on_destroy(pjmedia_port *this_port) {
    pj::socket_thread::thread_remove_port(this_port);
    return PJ_SUCCESS;
}

