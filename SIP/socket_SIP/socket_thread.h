#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H
#include <pjmedia/port.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace pj{
class socket_thread {
    public:
        static int socket_fd;
        static int socket_record_fd;
        static volatile int thread_quit_flag;
        static pjmedia_port* thread_play_port;
        static pjmedia_port* thread_record_port;
        static sockaddr_in* thread_record_addr;
        static sockaddr_in* thread_play_addr;
        static pj_pool_t* thread_pool;
        static pj_thread_t *thread_play;
        static void thread_pool_init();
        static void* thread_play_thread(void* input);
        static int create_play_thread(const char* listen_ip, int listen_port);
        static void exit_play_thread();
        static void thread_add_record_port(pjmedia_port* port, const char* udp_ip, int udp_port);
        static void thread_add_play_port(pjmedia_port* port);//TODO IMSI head?
        static void thread_remove_port(pjmedia_port* port);
        static void thread_record_frame(pjmedia_port* port, pjmedia_frame* frame);
};
}
#endif
