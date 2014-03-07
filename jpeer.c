#include "jpeer.h"


static jPeer *peer_alloc (jPeerManager *cm)
{
    jPeer *c = NULL;

    if (cm->num_peers) {
        c = (jPeer*)DC_list_get_object_at_index (&cm->peers, 0);
        DC_list_remove_object_at_index (&cm->peers, 0);
        cm->num_peers--;
    }

    return c;
}

static void peer_release (jPeerManager *cm, jPeer *c)
{
    DC_list_add (&cm->peers, (void*)c);
    cm->num_peers++;
}


BOOL PeerManagerInit (jPeerManager *cm, int maxpeers)
{
    int i;

    memset (cm, '\0', sizeof (jPeerManager));

    if (DC_list_init (&cm->peers, NULL) < 0) {
        return FALSE;
    }


    cm->peer_base = (jPeer*)calloc (maxpeers, sizeof (jPeer));
    if (cm->peer_base == NULL) {
        DC_list_destroy (&cm->peers);
        return FALSE;
    }

    for (i=0; i<maxpeers; i++) {
        DC_list_add (&cm->peers, (void*)&cm->peer_base[i]);
    }

    cm->num_peers = maxpeers;
    cm->new_peer  = peer_alloc;
    cm->release_peer = peer_release;
    return TRUE;
}

void PeerManagerUninit (jPeerManager *cm)
{
    free (cm->peer_base);
    DC_list_destroy (&cm->peers);
}
