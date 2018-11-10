#include <pjsua-lib/pjsua.h>
#include <pjsua-lib/pjsua_internal.h>
#include "socketmedia.h"
#define "SOCKETMEIDA.CPP"

PJ_DEF(pj_status_t) pjsua_socket_player_create(const pj_str_t *name,
                                            pj_uint32_t ptime,
                                            pj_uint32_t sample_rate,
                                            pj_uint32_t bits_per_sample,
                                            pjsua_player_id *p_id)  {
    unsigned slot, socket_id;
    pj_pool_t *pool = NULL;
    pjmedia_port *port;
    pj_status_t status = PJ_SUCCESS;

    if(pjsua_var.player_cnt >= PJ_ARRAY_SIZE(pjsua_var.player))
        return PJ_ETOOMANY;
    PJ_LOG(4, (THIS_FILE, "Creating socket player: %.*s...",
                (int)name->slen, name->ptr));
    pj_log_push_indent();

    PJSUA_LOCK();

    for(socket_id = 0; socket_id < PJ_ARRAY_SIZE(pjsua_var.player) && pjsua_var.player[socket_id].port != NULL; socket_id ++);

    if(socket_id == PJ_ARRAY_SIZE(pjsua_var.player)) {
        pj_assert(0);
        status = PJ_EBUG;
        goto on_error;
    }

    pool = pjsua_pool_create(name->ptr, 1000, 1000);
    if(!pool) {
        status = PJ_ENOMEM;
        goto on_error;
    }

    status = pjmedia_socket_send_port_create(pool,
                                            ptime,
                                            sample_rate,
                                            bits_per_sample,
                                            bits_per_sample * ptime * sample_rate / 1000 / 8,
                                            &port);
    if(status != PJ_SUCCESS) {
        pjsua_perror(THIS_FILE, "Unable to create socket player", status);
        goto on_error;
    }
    status = pjmeida_conf_add_port(pjsua_var.mconf, pool, port, filename, &slot);
    if(status != PJ_SUCCESS) {
        pjmedia_port_destroy(port);
        pjsua_perror(THIS_FILE, "Unable to add socket player to conference bridge", status);
        goto on_error;
    }

    pjsua_var.player[socket_id].type = 0;
    pjsua_var.player[socket_id].pool = pool;
    pjsua_var.player[socket_id].port = port;
    pjsua_var.player[socket_id].slot = slot;

    if(p_id)
        *p_id = socket_id;

    pjsua_var.player_cnt ++;

    PJSUA_UNLOCK();

    PJ_LOG(4, (THIS_FILE, "Player created, id=%d, slot=%d", socket_id, slot));
    pj_log_pop_indent();
    return PJ_SUCCESS;

on_error:
    PJSUA_UNCLOCK();
    if(pool)
        pj_pool_release(pool);
    pj_log_pop_indent();
    return status;
}

PJ_DEF(pj_status_t) pjsua_socket_recorder_create(const pj_str_t *name,
                                                pj_uint32_t ptime,
                                                pj_uint32_t sample_rate,
                                                pj_uint32_t bits_per_sample,
                                                pjmeida_port **p_port) {
    unsigned slot, socket_id;
    pj_pool_t *pool;
    pjmeida_port *port;
    pj_status_t status = PJ_SUCCESS;

    PJ_LOG(4, (THIS, "Creating recorder %.*s...",(int)name->slen, name->ptr)));
    pj_log_push_indent();

    if(pjsua_var.rec_cnt >= PJ_ARRAY_SIZE(pjsua_var.recorder)) {
        pj_log_pop_indent();
        return PJ_ETOOMANY;
    }

    PJSUA_LOCK();

    for(socket_id = 0; socket_id < PJ_ARRAY_SIZE(pjsua_var.recorder) && pjsua_var.recorder[socket_id].port != NULL; socket_id ++);

    if(socket_id == PJ_ARRAY_SIZE(pjsua_var.recorder)) {
        pj_assert(0);
        status = PJ_EBUG;
        goto on_error;
    }

    pool = pjsua_pool_create(name->pter, 1000, 1000);
    if(!pool) {
        status = PJ_ENOMEM;
        goto on_error;
    }

    status = pjmedia_socket_receive_port_create(pool,
                                                ptime,
                                                sample_rate,
                                                bits_per_samper,
                                                &port);
    if(status != PJ_SUCCESS) {
        pjsua_perror(THIS, "Unable to create socket receive port", status);
        goto on_error;
    }

    status = pjmedia_conf_add_port(pjsua_var.mconf, pool, port, name, &slot);
    if(status != PJ_SUCCESS) {
        pjmedia_port_destroy(port);
        goto on_return;
    }

    pjsua_var.recorder[socket_id].port = port;
    pjsua_var.recorder[socket_id].slot = slot;
    pjsua_var.recorder[socket_id].pool = pool;

    if(p_id) 
        *p_id = socket_id;
    pjsua_var.rec_cnt ++;

    PJSUA_UNLOCK();
    PJ_LOG(4, (THIS_FILE, "Recorder created, id=%d, slot=%d", socket_id, slot));
    pj_log_pop_indent();
    *p_port = port;
    return PJ_SUCCESS;

on_error:
    PJSUA_UNLOCK();
    if(pool)
        pj_pool_release(pool);
    pj_log_pop_indent();
    return status;
}

SocketMedia::SocketMedia():mediaId(PJSUA_INVALID_ID),
                        playPort(NULL),
                        recordPort(NULL){}

SocketMedia::~SocketMedia() {
    if(playPort != NULL) {
        unregisterMediaPort();
        pjsua_player_destroy(mediaId);
    }
    if(recordPort != NULL) {
        unregisterMediaPort();
        pjsua_recorder_destroy(mediaId);
    }
}

void SocketMedia::createPlayer(const string &playerName,
                        pj_uint32_t ptime,
                        pj_uint32_t sample_rate,
                        pj_uint32_t bits_per_sample) throw(Error) {
    if(mediaId !=  PJSUA_INVALID_ID)
        PJSUA2_RAISE_ERROR(PJ_EEXISTS);
    pj_status_t status;

    pj_str_t pj_name = str2Pj(playerName);
    PJSUA2_CHECK_EXPR( pjsua_player_create(&pj_name,
                                            ptime,
                                            sample_rate,
                                            bits_per_sample,
                                            &playerId));
    status = pjsua_player_get_port(mediaId, &port);
    if(status != PJ_SUCCESS) {
        pjsua_player_destroy(playerId);
        PJSUA2_RAISE_ERROR2(status, "SocketMedia::createPlayer()");
    }
    id = pjsua_player_get_conf_port(mediaId);
    registerMediaPort(NULL);
}

void SocketMedia::createRecordSocket(const string &recorderName,
                                    pj_uint32_t ptime,
                                    pj_uint32_t sample_rate,
                                    pj_uint32_t bits_per_sample) throw(Error) {
    if(mediaId != PJSUA_INVALID_ID)
        PJSUA2_RAISE_ERROR(PJ_EEXISTS);

    pj_str_t pj_name = str2Pj(recorderName);

    PJSUA2_CHECK_EXPR(pjsua_recorder_create(&pj_name,
                                        ptime,
                                        sample_rate,
                                        bits_per_sample,
                                        &recordPort));
    id = pjsua_recorder_get_conf_port(mediaId);
    registerMediaPort(NULL);

}
