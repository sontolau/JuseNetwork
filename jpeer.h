#ifndef _JPEER_H
#define _JPEER_H

#include "juse.h"
#include "jnetbuf.h"
#include "jinet.h"
#include "jnetbuf.h"


typedef struct _jPeer {
#ifdef OS_WINDOWS
#else
    int sock_fd;
#endif
    jINET  inet_info;
    //unsigned int trans_buf_size;
    //unsigned char *bytes;
    //DC_list_t  data_collector;
    jNetbuf  *netbuf;
} jPeer;

typedef struct _jPeerManager {
    int num_peers;
    jPeer *peer_base;
    DC_list_t peers;

    jPeer *(*new_peer) (struct _jPeerManager*);
    void (*release_peer) (struct _jPeerManager*, jPeer*);
} jPeerManager;

extern BOOL PeerManagerInit (jPeerManager *cm, int maxpeers);
extern void PeerManagerUninit (jPeerManager *cm);

#endif
