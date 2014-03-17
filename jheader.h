#ifndef _JUSE_HEADER_H
#define _JUSE_HEADER_H

#include "jerror.h"

#define JUSE_ID   0x4A555345

#define JCOM_CONN        1
#define JCOM_PING        2
#define JCOM_TRANS       3
#define JCOM_DISCONN     4

typedef struct _jTransHeader {
    unsigned int identifier;
    unsigned short command;
    unsigned short flags;
    jError         error;
    unsigned int   PID;
    unsigned int   MID;
    unsigned int   length;
} jTransHeader;

#define SZTRANSHDR   (sizeof (jTransHeader))
#define JUSE_REPLY(hdr) ((hdr).flags & 0x8000)
#define JUSE_ARG(hdr)   ((hdr).flags & 0x7FFF)

#endif //_JUSE_HEADER_H
