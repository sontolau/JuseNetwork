#include "jutilities.h"


void J_set_module_alive_interval (jModule *mod, int sec, void (*thrd_func)(jModule*))
{
    jModuleDescriptor *desc;

    desc = mod->mod_descriptor;
    if (desc) {
        desc->alive_interval = sec;
        desc->thread_func    = thrd_func;
    }
}


