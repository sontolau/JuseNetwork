#include "jpayload.h"

typedef struct _Juse {
    int sock_fd;
    unsigned int addr;
    unsigned short port;
    unsigned int max_trans_size;
    unsigned int udp_pkt_size;
} Juse_t;


static int tcp_connect (Juse_t *juse)
{
    struct sockaddr_in skaddr;
    jPayload   payload;

    skaddr.sin_family = AF_INET;
    skaddr.sin_port   = htons (juse->port);
    skaddr.sin_addr.s_addr = juse->addr;

    juse->sock_fd = socket (AF_INET, SOCK_STREAM, 0);
    if (juse->sock_fd < 0) {
        return -1;
    }

    if (connect (juse->sock_fd, (struct sockaddr*)&skaddr, sizeof (skaddr)) < 0) {
        close (juse->sock_fd);
        return -1;
    }

    
}

static int udp_connect (Juse_t *juse)
{
    struct sockaddr_in skaddr;

    skaddr.sin_family = AF_INET;
    skaddr.sin_port   = htons (juse->port);
    skaddr.sin_addr.s_addr = juse->addr;

    juse->sock_fd = socket (AF_INET, SOCK_DGRAM, 0);
    if (juse->sock_fd < 0) {
        return -1;
    }

    
}

HJUSE J_Connect (const char *proto,
                 const char *domain,
                 unsigned short port)
{
    Juse_t *juse;
    struct hostent *ent;
    struct sockaddr_in skaddr;

    juse = (Juse_t*)calloc (1, sizeof (Juse_t));
    if (juse == NULL) {
err_quit:
        if (juse) {
            free (juse);
        }
        return NULL;
    }

    ent = gethostbyname (domain);
    if (ent == NULL) {
        goto err_quit;
    } else {
        juse->addr = *(unsigned int*)ent->h_addr;
        juse->port = port;
        free (ent);
    }

    if (!strcmp (proto, "TCP")) {
        if (tcp_connect (&juse) < 0) {
            goto err_quit;
        }
    } else if (!strcmp (proto, "UDP")) {
        if (udp_connect (&juse) < 0) {
            goto err_quit;
        }
    } else {
        goto err_quit;
    }

    return (HJUSE)juse;
}
