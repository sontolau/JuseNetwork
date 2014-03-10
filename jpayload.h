#ifndef _JPAYLOAD_H
#define _JPAYLOAD_H

#include "jerror.h"

#define ESYSTEM  0x0001
#define ESYS_SYSTEM_CALL 1
#define ESYS_BUSY        2
#define ESYS_NO_RESOURCE 3


#define JUSE_MAGIC "JUSE"


enum {
   JuseComSet = 1,
   JuseComTrans,
};

typedef struct _JusePayload {
    unsigned char magic[4];
    unsigned int  version;
    unsigned int  flags;
    jErrorStatus  status;
    unsigned int  mid;
    unsigned int  tid;
    unsigned int  size;
} JusePayload;
#define SZPAYLOD  (sizeof (JusePayload))

#define JUSE_REQUEST(flags) (!(flags & 0x80000000))
#define JUSE_COM(flags)     ((flags & 0x70000000) >> 28)
#define JUSE_COM_VALUE(flags) (flags & 0x0FFFFFFF)


#endif
