#include "jpayload.h"
#include "jerror.h"

static jPayload EOKPayloads = {
    [0] = {
        .magic = JUSE_MAGIC,
        .versiion = 1,
        .flags    = 0,
        .status   = {EOK, 0},
        .mid      = 0,
        .tid      = 0,
        .size     = 0,
    }
};


static jPayload   __ESystemPayloads[] = {
    [ESYS_SYSTEM] = {
        .magic = JUSE_MAGIC,
        .version = 1,
        .flags   = 0,
        .status  = {ESYSTEM, ESYS_SYSTEM},
        .mid     = 0,
        .tid     = 0,
        .size    = 0,
    },
    [ESYS_INVALID_REQUEST] = {
        .magic = JUSE_MAGIC,
        .version = 1,
        .flags   = 0,
        .status  = {ESYSTEM, ESYS_INVALID_REQUEST},
        .mid     = 0,
        .tid     = 0,
        .size    = 0,
    },

    [ESYS_OVER_BUFSIZE] = {
        .magic = JUSE_MAGIC,
        .versiion = 1,
        .flags    = 0,
        .status   = {ESYSTEM, ESYS_OVER_BUFSIZE},
        .mid      = 0,
        .tid      = 0,
        .size     = 0,
    },
    [ESYS_TRANS_TIMEO] = {
        .magic = JUSE_MAGIC,
        .version = 1,
        .flags   = 0,
        .status  = {ESYSTEM, ESYS_TRANS_TIMEO},
        .mid     = 0,
        .tid     = 0,
        .size    = 0,
    },
    [ESYS_BUSY] = {
        .magic = JUSE_MAGIC,
        .version = 1,
        .flags   = 0,
        .status  = {ESYSTEM, ESYS_BUSY},
        .mid     = 0,
        .tid     = 0,
        .size    = 0,
    }
};

static jPayload __EModulePayloads[] = {
    [EMOD_INVALID_MODULE] = {
        .magic = JUSE_MAGIC,
        .versiion = 1,
        .flags    = 0,
        .status   = {EMODULE, EMOD_INVALID},
        .mid      = 0,
        .tid      = 0,
        .size     = 0,

    },
    [EMOD_DISABLED] = {
        .magic = JUSE_MAGIC,
        .versiion = 1,
        .flags    = 0,
        .status   = {EMODULE, EMOD_DISABLED},
        .mid      = 0,
        .tid      = 0,
        .size     = 0,
    }

};

jPayload **ErrorPayloads()
{
    int i;
    jPayload **payloads = NULL;

    payloads = (jPayload**)calloc (3, sizeof (jPayloads*));
    if (payloads) {
        payloads[EOK]     = &__EOKPayloads;
        payloads[ESYSTEM] = &__ESystemPayloads[0];
        payloads[EMODULE] = &__EModulePayloads[0];
    }

    return payloads;
}
