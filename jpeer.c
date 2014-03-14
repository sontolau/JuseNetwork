#include "jpeer.h"


static jPeer *peer_alloc (jPeerManager *cm)
{
    jPeer *c = NULL;
    char strid[10];

    if (cm->num_peers) {
        c = (jPeer*)DC_list_get_object_at_index (&cm->peers, 0);
        if (c) {
            c->pid = (++cm->base_id);
            sprintf (strid, "%lu", c->pid);
            DC_dict_add_object_with_key (&cm->engaged_peers, (char*)strid, (void*)c);
        }
        DC_list_remove_object_at_index (&cm->peers, 0);
        cm->num_peers--;
    }

    return c;
}

static void peer_release (jPeerManager *cm, jPeer *c)
{
    char strid[10];

    sprintf (strid, "%lu", c->pid);
    DC_dict_remove_object_with_key (&cm->engaged_peers, (char*)strid);
    DC_list_add (&cm->peers, (void*)c);
    cm->num_peers++;
}

static jPeer *peer_find (jPeerManager *cm, unsigned int pid)
{
    char strid[10];

    sprintf (strid, "%lu", pid);
    return DC_dict_get_object_with_key (&cm->engaged_peers, (char*)strid);
}

BOOL PeerManagerInit (jPeerManager *cm, int maxpeers)
{
    int i;

    memset (cm, '\0', sizeof (jPeerManager));

    if (DC_list_init (&cm->peers, NULL) < 0) {
        return FALSE;
    }


    if (DC_dict_init (&cm->engaged_peers, NULL) < 0) {
        DC_list_destroy (&cm->peers);
        return FALSE;
    }

    cm->peer_base = (jPeer*)calloc (maxpeers, sizeof (jPeer));
    if (cm->peer_base == NULL) {
        DC_list_destroy (&cm->peers);
        DC_dict_destroy (&cm->engaged_peers);
        return FALSE;
    }

    for (i=0; i<maxpeers; i++) {
        DC_list_add (&cm->peers, (void*)&cm->peer_base[i]);
    }
    cm->base_id   = (unsigned int)time (NULL);
    cm->num_peers = maxpeers;
    cm->new_peer  = peer_alloc;
    cm->release_peer = peer_release;
    cm->get_peer = peer_find;
    return TRUE;
}

void PeerManagerUninit (jPeerManager *cm)
{
    free (cm->peer_base);
    DC_list_destroy (&cm->peers);
}
