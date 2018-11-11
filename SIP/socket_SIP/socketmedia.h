#ifndef SOCKETMEDIA_H
#define SOCKETMEDIA_H
#include <pjsip.h>
#include <pjmedia.h>
#include <pj/ctype.h>
#include <pjsua2/media.hpp>
#include <pjsua2/types.hpp>
#include <pjsua2/endpoint.hpp>

PJ_DECL(pj_status_t) socket_player_create(const pj_str_t *name,
                                            pj_uint32_t ptime,
                                            pj_uint32_t sample_rate,
                                            pj_uint32_t bits_per_sample,
                                            pjsua_player_id *p_id);

PJ_DECL(pj_status_t) pjsua_socket_recorder_create(const pj_str_t *name,
                                            pj_uint32_t ptime,
                                            pj_uint32_t sample_rate,
                                            pj_uint32_t bits_per_sample,
                                            pjmedia_port **p_port);

namespace pj {

using std::string;
class SocketMedia: public AudioMedia {
    public:
        SocketMedia();
        virtual ~SocketMedia();
        void createPlayer(const string &playerName,
                            pj_uint32_t ptime,
                            pj_uint32_t sample_rate,
                            pj_uint32_t bits_per_sample) throw(Error);
        void createRecorder(const string &recorderName,
                            pj_uint32_t ptime,
                            pj_uint32_t sample_rate,
                            pj_uint32_t bits_per_sample) throw(Error);
    private:
        int mediaId;
        pjmedia_port *playPort;
        pjmedia_port *recordPort;

};
}
#endif
