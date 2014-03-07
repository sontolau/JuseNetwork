#ifndef _JERROR_H
#define _JERROR_H

#include "jmodule.h"

#ifdef OS_WINDOWS
#else
#include <errno.h>
#define JERROR_CODE   errno
#endif

typedef struct _jErrorStatus {
    unsigned short eclass;
    unsigned short ecode;
} jErrorStatus;

#endif
