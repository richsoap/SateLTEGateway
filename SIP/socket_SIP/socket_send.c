#include "socket_port.h"
#include <pj/pool.h>
#include <pj/log.h>
#include <pj/string.h>
#include <pj/assert.h>
#include <pjmedia/errno.h>
#include <pjmedia/wave.h>

#define THIS_FILE "socket_send.cpp"

#define SIGNATURE PJMEDIA_SIG_PORT_WAV_PLAYER

struct socket_send_port {
    pjmedia_port port;
    pj_uint32_t buffer_size;
    pj_uint32_t data_len;
    pj_pool_t *pool;
    char *buffer;
};

static pj_status_t socket_get_frame(pjmedia_port *this_port, pjmedia_frame* frame);
static pj_status_t socket_on_destroy(pjmedia_port *this_port);

static struct socket_send_port *create_socket_port(pj_pool_t *pool) {
    const pj_str_t name = pj_str("socket");
    struct socket_send_port *port;

    port = PJ_POOL_ZALLOC_T(pool, struct socket_send_port);
    if(!port)
        return NULL;

    pjmedia_port_info_init(&port->port.info, &name, SIGNATURE, 8000, 1, 16, 80);// TODO: What these default paramaters for?
    port->port.get_frame = &socket_get_frame;
    port->port.on_destroy = &socket_on_destroy;
    port->pool = pool;
    return port;
}

PJ_DEF(pj_status_t) pjmedia_socket_send_port_create(pj_pool_t *pool,
                                                    pj_uint32_t ptime,/*ms*/
                                                    pj_uint32_t sample_rate,
                                                    pj_uint32_t bits_per_sample,
                                                    pj_ssize_t buffer_size,
                                                    pjmedia_port **p_port) {
   struct socket_send_port *sock_port;
   pj_uint32_t samples_per_frame;
   pj_str_t name;
   PJ_ASSERT_RETURN(pool && p_port, PJ_EINVAL);

   sock_port = create_socket_port(pool);
   if(!sock_port)
       return PJ_ENOMEM;

   /*Update port info*/
   if(ptime == 0)
       ptime = 20;
   if(sample_rate == 0)
       sample_rate = 8000;
   name = pj_str("socket");
   samples_per_frame = ptime * sample_rate / 1000;
   pjmedia_port_info_init(&sock_port->port.info,
                            &name,
                            SIGNATURE,
                            sample_rate,
                            1,
                            bits_per_sample,
                            samples_per_frame);
   
    /*Alloc buffer*/
   sock_port->buffer_size = (pj_uint32_t) buffer_size;
   sock_port->buffer = (char*) pj_pool_alloc(pool, sock_port->buffer_size);
   if(!sock_port->buffer)
       return PJ_ENOMEM;
   *p_port = &sock_port->port;
   PJ_LOG(4, (THIS_FILE,"Socket Port Built\n"));
   return PJ_SUCCESS;

}

static pj_status_t socket_get_frame(pjmedia_port *this_port, pjmedia_frame* frame) {
    struct socket_send_port* sock_port = (struct socket_send_port*) this_port;
    pj_size_t frame_size;
    
    pj_assert(sock_port->port.info.signature == SIGNATURE);
    pj_assert(frame->size <= sock_port->buffer_size);
    pj_memcpy(frame->buf, sock_port->buffer, sock_port->data_len);
    return PJ_SUCCESS;
}

static pj_status_t socket_on_destroy(pjmedia_port *this_port) {
    //TODO:remove_send_port(this_port);
    return PJ_SUCCESS;
}
