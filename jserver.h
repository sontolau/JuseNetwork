#ifndef _JUSE_SERVER_H
#define _JUSE_SERVER_H

#include "juse.h"
#include "jpeer.h"

typedef struct _jServer {
    jINET  inet_info;
    DC_list_t   sock_peers;
    DC_dict_t   conn_peers;
    int         ping_interval;
    int         max_peers;
    int max_trans;


    jPeerManager mem_peer;
    jNetbufManager mem_netbuf;

    int (*core_handler) (int status, jPeer *peer, void *data);
    void *data;
} jServer;


extern int ServerCreate (jServer *serv, int proto, const char *addr, unsigned short port);
extern int ServerSetOptions (jServer *serv, int pi, int max_peers, int max_trans);
extern void ServerRun (jServer *serv, );
extern void ServerClose (jServer *serv);

#endif
