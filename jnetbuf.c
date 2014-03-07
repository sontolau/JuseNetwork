#include "jnetbuf.h"

static jNetbuf *buffer_alloc (jNetbufManager *mgr, unsigned int len)
{
    jNetbuf *buf;

    if (len > mgr->unit_size || mgr->buffers.count == 0) {
        return NULL;
    }

    buf = (jNetbuf*)DC_list_get_object_at_index (&mgr->buffers, 0);
    DC_list_remove_object_at_index (&mgr->buffers, 0);

    return buf;
}

static void buffer_free (jNetbufManager *mgr, jNetbuf *buf)
{
    DC_list_add (&mgr->buffers, (void*)buf);
}

BOOL NetbufManagerInit (jNetbufManager *nm, unsigned int pkt_size, int num)
{
    int i;
    register unsigned char *byteptr;

    memset (nm, '\0', sizeof (jNetbufManager));

    nm->num_buffers = num;
    nm->unit_size   = pkt_size;

    if (DC_list_init (&nm->buffers, NULL) < 0) {
        return FALSE;
    }

    byteptr = nm->data_buffer = calloc (num, pkt_size);
    if (nm->data_buffer == NULL) {
err_quit:
        if (nm->data_buffer) free (nm->data_buffer);
        if (nm->net_buffer)  free (nm->net_buffer);
        DC_list_destroy (&nm->buffers);

        return FALSE;
    }

    nm->net_buffer = (jNetbuf*)calloc (num, sizeof (jNetbuf));
    if (nm->net_buffer == NULL) {
        goto err_quit;
    }

    for (i=0; i<num; i++) {
        memset (&nm->net_buffer[i], '\0', sizeof (jNetbuf));
        nm->net_buffer[i].data = byteptr;
        DC_list_add (&nm->buffers, (void*)&nm->net_buffer[i]);
        byteptr += pkt_size;
    }

    nm->alloc_buffer = buffer_alloc;
    nm->free_buffer  = buffer_free;

    return TRUE;
}

void NetbufManagerUninit (jNetbufManager *nm)
{
    free (nm->data_buffer);
    free (nm->net_buffer);
    DC_list_destroy (&nm->buffers);
}
