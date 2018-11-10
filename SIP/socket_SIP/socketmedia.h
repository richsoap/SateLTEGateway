#ifndef SOCKETMEDIA_H
#define SOCKETMEDIA_H

static pj_status_t pjsua_socket_player_create(const pj_str_t *name,
                                            pj_uint32_t ptime,
                                            pj_uint32_t sample_rate,
                                            pj_uint32_t bits_per_sample,
                                            pjsua_player_id *p_id);

static pj_status_t pjsua_socket_recorder_create(const pj_str_t *name,
                                            pj_uint32_t ptime,
                                            pj_uint32_t sample_rate,
                                            pj_uint32_t bits_per_sample,
                                            pjmedia_port **p_port);
// TODO not static

class SocketMedia: public AudioMedia {
    public:
        SocketMedia();
        virtual ~SocketMedia();
        void createPlaySocket(const string *playerName,
                            pj_uint32_t ptime,
                            pj_uint32_t sample_rate,
                            pj_uint32_t bits_per_sample);
        void createRecordSocket(const string &recorderName,
                            pj_uint32_t ptime,
                            pj_uint32_t sample_rate,
                            pj_uint32_t bits_per_sample);
    private:
        int mediaId;
        pjmedia_port *playPort;
        pjmedia_port *recordPort;

}
#endif
