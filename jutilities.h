#ifndef _JUTILITIES_H
#define _JUTILITIES_H

#include "jmodule.h"

extern void J_set_module_statistics (jModule *mod, int alive_sec, void (*thrd_func)(jModule*));
#endif
