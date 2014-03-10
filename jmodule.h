#ifndef _JMODULE_H
#define _JMODULE_H
#include "juse.h"

#include "dict.h"
#include "list.h"

#define __user

#define MOD_EVENT_START 1
#define MOD_EVENT_RUN   2
#define MOD_EVENT_PING  3
#define MOD_EVENT_STOP  4


struct _jModuleDescriptor;
typedef struct _jModule {
    unsigned int __user mod_version;
    struct _jModuleDescriptor *mod_descriptor;
    void __user *mod_data;

    BOOL __user (*mod_init) (struct _jModule *module);

    void __user (*mod_handler) (struct _jModule *module, int event, void *arg);

    void __user (*mod_exit) (struct _jModule *module);
} jModule;

#define JMODULE_EXPORT_SYMBOL  "__juse_module"

#define JMODULE_DEFINE(_version, _init, _handler, _uninit) \
	jModule __juse_module = { \
            .mod_version = _version, \
            .mod_init    = _init, \
            .mod_handler = _handler, \
            .mod_uninit  = _uninit, \
        };



#define EMODULE  (1<<2)

#define EMODULE_NOT_FOUND  1
#define EMODULE_DISABLED   2
#define EMODULE_BUSY       3

enum {
    ModuleStatusIdle = 0,
    ModuleStatusBusy,
};

typedef struct _jModuleDescriptor {
    char *name;
    unsigned int mid;
    unsigned int status;
    BOOL  enabled;
    int  alive_interval;
    void *handle;
    void (*thread_func) (jModule*);
} jModuleDescriptor;


typedef struct _jModuleManager {
    DC_dict_t  modules;
    DC_list_t  mids;
    BOOL (*load_module) (struct _jModuleManager *self, unsigned int mid, const char *name);  
    void (*unload_module) (struct _jModuleManager *self, unsigned int mid);
    jModule *(*get_module) (struct _jModuleManager *self, unsigned int mid);
    void (*clear_all_modules) (struct _jModuleManager *self);
} jModuleManager;


extern BOOL ModuleManagerInit (jModuleManager *manager);
extern void ModuleManagerUninit (jModuleManager *manager);

#endif //_JMODULE_H
