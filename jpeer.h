#ifndef _JPEER_H
#define _JPEER_H

#include "juse.h"
#include "jnetbuf.h"


typedef struct _jPeer {
    unsigned pid;
    jINET  inet_info;
    //unsigned int mid;
    unsigned int conn_time;
    BOOL         conn_flag;
    jNetbuf  *netbuf;
} jPeer;

typedef struct _jPeerManager {
    int num_peers;
    unsigned int base_id;
    jPeer *peer_base;
    DC_list_t peers;
    DC_dict_t engaged_peers;

    jPeer *(*get_peer) (struct _jPeerManager*, unsigned int pid);
    jPeer *(*new_peer) (struct _jPeerManager*);
    void (*release_peer) (struct _jPeerManager*, jPeer*);
} jPeerManager;

extern BOOL PeerManagerInit (jPeerManager *cm, int maxpeers);
extern void PeerManagerUninit (jPeerManager *cm);

#endif
