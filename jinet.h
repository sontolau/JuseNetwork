#ifndef _JINET_H
#define _JINET_H

enum {
    INETProtocolUDP = 1,
    INETProtocolTCP,
};

typedef struct _jINET {
    int proto;
    unsigned int addr;
    unsigned short port;
} jINET;

#endif
