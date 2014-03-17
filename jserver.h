#ifndef _JUSE_SERVER_H
#define _JUSE_SERVER_H

#include "juse.h"
#include "jpeer.h"
#include "jerror.h"
#include "jsig-event.h"

typedef struct _jServer {
    jINET  inet_info;
    DC_list_t   sock_peers;
    DC_dict_t   conn_peers;
    int         ping_interval;
    int         time_wait;
    int         max_peers;
    int         max_trans;
    unsigned int ping_count;
    volatile int quit_flag;
    pthread_rwlock_t rwlock;
    jPeerManager peer_manager;
    jNetbufManager nb_manager;

    jSignal       signal;
    pthread_t     run_thread;
    void *private_data;
    void (*timeout) (struct _jServer*, void*);
    int (*connect) (struct _jServer*, jPeer*, void*, jError*);
    int (*transaction) (struct _jServer*,jPeer*, void*, jError*);
    int (*disconnect) (struct _jServer*, jPeer*, void*, jError*);
} jServer;


extern int ServerCreate (jServer *serv, int proto, const char *addr, unsigned short port);
extern int ServerRun (jServer *serv, int thread);

extern void ServerQuit (jServer *serv, int wait);
extern void ServerClose (jServer *serv);

#endif
