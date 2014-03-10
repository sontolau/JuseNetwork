#ifndef _JNETBUF_H
#define _JNETBUF_H

#include "juse.h"
#include "list.h"
#include "jpayload.h"

typedef struct _jNetbuf {
    unsigned int seqno;
    unsigned int length;
    jPayload     *payload;
    unsigned char *data;
    union {
        int           intval;
        void          *pointer;
    };
} jNetbuf;

#define NETBUF_DATA_LEN(nb) (nb->length + SZPAYLOAD);

typedef struct _jNetbufManager {
    int num_buffers;
    unsigned int unit_size;
    unsigned char *data_buffer;
    jNetbuf *net_buffer;
    DC_list_t  buffers;

    jNetbuf *(*alloc_buffer) (struct _jNetbufManager *self, unsigned int size);
    void (*free_buffer) (struct _jNetbufManager *self, jNetbuf*);
} jNetbufManager;


extern BOOL NetbufManagerInit (jNetbufManager *mgr, unsigned int pkt_len, int num);
extern void NetbufManagerUninit (jNetbufManager *mgr);

#endif
