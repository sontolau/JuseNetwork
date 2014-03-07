#include "jframework.h"


int RunModulesInProcess (jFrameContext *fc)
{
}

int LoadModules (jFrameContext *fc)
{
    int i;
    char modpath[255] = {0};
    jMetaModule *mmod;
    jModule *module;

    if (!ModuleManagerInit (&fc->module_manager)) {
        return -1;
    }


    for (i=0; i<fc->config.module.num_modules; i++) {
        snprintf (modpath, sizeof (modpath)-1, "%s/%s/%s",
                  fc->config.general.chdir,
                  fc->config.module.moddir,
                  fc->config.module.modules[i].mod_name);
        if (!fc->module_manager.load_module (&fc->module_manager,
                                             fc->config.module.modules[i].mod_id,
                                               modpath)) {
            jerr ("load module[%s] failed.\n", fc->config.module.modules[i].mod_name);
        } else {
            module = fc->module_manager.get_module (&fc->module_manager,
                                                  fc->config.module.modules[i].mod_id);
            if (module && (mmod = (jMetaModule*)calloc (1, sizeof (jMetaModule)))) {
                mmod->module = module;
                mmod->unix_sock_path = tmpnam (NULL, "JN-UNIX");

                sprintf (modpath, "%lu", fc->config.module.modules[i].mod_id);
                DC_dict_add_object_with_key (&fc->proc_modules, (char*)modpath, mmod);                
            } else {
                fc->module_manager.unload_module (&fc->module_manager,
                                                  fc->config.module.modules[i].mod_id);
            }
        }
    }
    return 0;
}

int LoadConfig (jConfig *cfg, const char *path) 
{
    SetDefaultConfig (cfg);

    if (!ConfigLoad (cfg, path)) {
        return -1;
    }

    if (CheckConfig (cfg) < 0) {
        return -1;
    }

    return 0;
}

void RunModulesInProcess (jFrameContext *fc)
{
    
}

void JuseRun (jFrameContext *fc)
{
    RunModulesInProcess (fc);
}

int JuseInit (jFrameContext *fc, const char *cfg)
{
    if (DC_dict_init (&fc->proc_modules, NULL) < 0) {

    }

    if (LoadConfig (&fc->config, cfg) < 0) {

    }

    if (LoadModules (fc) < 0) {

    }

    return 0;
}


int JuseMain (int argc, const char *argv[], JuseDelegate *delegate)
{
    char *cfg_path;
    jFrameContext *fc;

    if (JuseInit (fc, const char *cfg)) {

    }

    JuseRun (fc);

    JuseUninit (fc);

    return 0;
}
