#ifndef SOCKET_PORT_H
#define SOCCKET_PORT_H

#include <pjmedia/port.h>
#include <pj/pool.h>
#include <pj/log.h>

PJ_BEGIN_DECL

PJ_DECL(pj_status_t) pjmedia_socket_send_port_create(pj_pool_t *pool,
                                                pj_uint32_t ptime,
                                                pj_uint32_t sample_rate,
                                                pj_uint32_t bits_per_sample,
                                                pj_ssize_t buffer_size,
                                                pjmedia_port **p_port);

PJ_DECL(pj_status_t) pjmedia_socket_receive_port_create(pj_pool_t *pool,
                                                pj_uint32_t ptime,
                                                pj_uint32_t sample_rate,
                                                pj_uint32_t bits_per_sample,
                                                pjmedia_port **p_port);

PJ_END_DECL

#endif
