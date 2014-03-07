#include "jmodule.h"

#ifdef OS_WINDOWS
#else
#include <dlfcn.h>
#endif


static BOOL module_load (jModuleManager *mgr, unsigned int mid, const char *name)
{
    char strid[10] = {0};
    jModuleDescriptor *desc;
    jModule           *module;

    
    desc = (jModuleDescriptor*)calloc (1, sizeof (jModuleDescriptor));
    if (desc) {
        desc->name = (char*)name;
        desc->mid  = mid;
        desc->status = ModuleStatusIdle;
        desc->enabled= TRUE;
#ifdef OS_WINDOWS
#else
        desc->handle = dlopen (name, RTLD_LAZY);
        if (desc->handle == NULL) {
load_failed:
            if (desc->handle) {
                dlclose (desc->handle);
            }
            free (desc);
            return FALSE;
        }

        module = (jModule*)dlsym (desc->handle, JMODULE_EXPORT_SYMBOL);
        if (module == NULL) {
            jerr ("No correct symbol exported in module:%s\n", name);
            goto load_failed;
        }
#endif
    
        module->mod_descriptor = desc;
        if (module->mod_init && !module->mod_init (module)) {
            jerr ("The module[%s] initailized failed, unload now.\n", name);
            goto load_failed; 
        }

        snprintf (strid, sizeof (strid) - 1, "%u", mid);
        DC_dict_add_object_with_key (&mgr->modules, (char*)strid, (void*)module);
        DC_list_add (&mgr->mids, (void*)mid);
    }

    return TRUE;
}

static void module_unload (jModuleManager *mgr, unsigned int mid)
{
    char strid[10] = {0};
    jModule *module;

    snprintf (strid, sizeof (strid) - 1, "%u", mid);
    module = DC_dict_get_object_with_key (&mgr->modules, (char*)strid);
    if (module) {
        if (module->mod_exit) {
            module->mod_exit (module);
        }
        DC_dict_remove_object_with_key (&mgr->modules, (char*)strid);
        DC_list_remove_object (&mgr->mids, (void*)mid);
#ifdef OS_WINDOWS
#else
        dlclose (module->mod_descriptor->handle);
        free (module->mod_descriptor);
#endif
    }
}

static jModule *module_get (jModuleManager *mgr, unsigned int mid)
{
    char strid[10];

    snprintf (strid, sizeof (strid) - 1, "%u", mid);
    return (jModule*)DC_dict_get_object_with_key (&mgr->modules, (char*)strid);
}

static void module_clear_all (jModuleManager *mgr)
{
    unsigned int mid;

    while (mgr->mids.count) {
        mid = (unsigned int)DC_list_get_object_at_index (&mgr->mids, 0);
        mgr->unload_module (mgr, mid);
        DC_list_remove_object_at_index (&mgr->mids, 0);
    }
}

BOOL ModuleManagerInit (jModuleManager *manager)
{
    int ret;

    ret = DC_dict_init (&manager->modules, NULL);
    if (ret < 0) {
        return FALSE;
    }

    ret = DC_list_init (&manager->mids, NULL);
    if (ret < 0) {
        DC_dict_destroy (&manager->modules);
        return FALSE;
    }

    manager->load_module   = module_load;
    manager->unload_module = module_unload;
    manager->get_module    = module_get; 
    manager->clear_all_modules = module_clear_all;
    return TRUE;
}


void jModuleManagerUninit (jModuleManager *manager)
{
    manager->clear_all_modules (manager);

    DC_dict_destroy (&manager->modules);
    DC_list_destroy (&manager->mids);
}
