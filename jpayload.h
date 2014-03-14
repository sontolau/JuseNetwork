#ifndef _JPAYLOAD_H
#define _JPAYLOAD_H

#include "jerror.h"

#define ESYSTEM  0x0001
#define ESYS_SYSTEM_CALL 1
#define ESYS_BUSY        2
#define ESYS_NO_RESOURCE 3


#define JUSE_MAGIC "JUSE"


enum {
   jComGetOption = 1,
   jComSetOption = 2,
   jComTrans
};

enum {
   jOptionTransSize = 1,
   jOptionUDPPacketSize = 2,
};

typedef struct _jPayload {
    unsigned int magic;
    unsigned int  version;
    unsigned int  flags;
    jErrorStatus  status;
    unsigned int  mid;
    unsigned int  tid;
    unsigned int  size;
} jPayload;
#define SZPAYLOD  (sizeof (JusePayload))

#define J_COMMAND(flags)  ((flags & 0xF0000000) >> 28)
#define J_OPTION(flags)   ((flags & 0x0F000000) >> 24)
#define J_ARGUMENT(flags) (flags & 0x00FFFFFF)




#define J_SET_PAYLOAD(pay, com, opt, value) ((pay).flags = ((com & 0xF) << 28) |\
                                                           ((opt & 0xF) << 24) |\
                                                           value)

#endif
