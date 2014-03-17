#ifndef _JERROR_H
#define _JERROR_H

#include "jmodule.h"

#ifdef OS_WINDOWS
#else
#include <errno.h>
#define JERROR_CODE   errno
#endif

typedef struct _jError {
    unsigned short eclass;
    unsigned short ecode;
} jError;

#define EOK  0x0000

#define ESYSTEM  0x0001
#define ESYS_SYSTEM 1
#define ESYS_BUSY        2
#define ESYS_OVER_BUFSIZE 4
#define ESYS_TRANS_TIMEO 5
#define ESYS_INVALID_REQUEST 6

#endif
