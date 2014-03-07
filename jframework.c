#include "jframework.h"


int RunProcInThread (void (*proc)(void*), void *data)
{
    pthread_t  pt;

    if (pthread_create (&pt, NULL, &proc, data) < 0) {
        return -1;
    }

    return 0;
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

void SetDefaultConfig (jConfig *cfg)
{
    cfg->general.proto = INETProtocolUDP;
    cfg->general.addr  = INADDR_ANY;
    cfg->general.port  = 3040;
    cfg->general.chdir = "/";
    cfg->general.log   = "/var/log/juse.log";
    cfg->general.unix_path = "/tmp/juse-sock.unix";

    
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

int ConnectIOServer (jFrameContext *fc, jMetaModule *module, int retry)
{
    struct timeval tm_wait = {1, 0};
    struct sockaddr_un un;
    int ret;

    module->io_sock = socket (AF_LOCAL, SOCK_STREAM, 0);
    if (module->io_sock < 0) {
        return -1;
    }

    setsockopt (module->io_sock, SOL_SOCKET, SO_SNDTIMEO, &tm_wait, sizeof (tm_wait));
    setsockopt (module->io_sock, SOL_SOCKET, SO_RCVTIMEO, &tm_wait, sizeof (tm_wait));

    memset (&un, '\0', sizeof (un));

    un.sun_family = AF_LOCAL;
    strcpy (un.sun_path, fc->config.general.unix_path);

    do {
        ret = connect (module->io_sock, (struct sockaddr*)&un, sizeof (un));
        if (ret == 0) {
            break;
        }

        retry--;
    } while (retry && errno == ETIMEOUT);

    return 0;
}

void RunModuleInProcess (jFrameContext *fc, jMetaModule *module)
{
    if (ConnectIOServer (fc, module, 4) < 0) {

    }

    MoudleProcCore (fc, module);

    CleanModuleProc (fc, module);
}

void RunModules (jFrameContext *fc)
{
    int i;
    char strmid[50];
    jMetaModule *mmod;
    pid_t pid;

    for (i=0; i<fc->config.module.num_modules; i++) {
        sprintf (strmid, "%lu", fc->config.module.modules[i].mod_id);
        mmod = DC_dict_get_object_with_key (&fc->proc_modules, (char*)strmid);
        if (mmod) {
            pid = fork ();
            if (pid < 0) {

            } else if (pid == 0) {
                RunModuleInProcess (fc, mmod);
            } else {
            }
        }
    }
    
}

int CreateInetServer (int proto, unsigned int addr, unsigned short port)
{
    int sock;
    struct sockaddr_in addr;
    int flag = 1;
    unsigned int szsockbuf = 0xFFFF;

    if (proto == INETProtocolTCP) {
        sock = socket (AF_INET, SOCK_STREAM, 0);
    } else if (proto == INETProtocolUDP) {
        sock = socket (AF_INET, SOCK_DGRAM, 0);
    }

    setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (flag));
    setsockopt (sock, SOL_SOCKET, SO_SNDBUF, &szsockbuf, sizeof (int));
    setsockopt (sock, SOL_SOCKET, SO_RCVBUF, &szsockbuf, sizeof (int));

    
    addr.sin_family = AF_INET;
    addr.sin_port   = htons (port);
    addr.sin_addr.s_addr = addr;

    if (bind (sock, (struct sockaddr*)&addr, sizeof (addr)) < 0) {
        close (sock);
        return -1;
    }

    if (proto == INETProtocolTCP) {
        listen (sock, 100);
    }
    return sock;
}

int CreateDataServer (jFrameContext *fc)
{
    fc->data_sock = CreateInetServer (fc->config.general.proto, 
                                      fc->config.general.addr,
                                      fc->config.general.port);
    if (fc->data_sock < 0) {
        return -1;
    }

    return 0;
}

int CreateRMIServer (jFrameContext *fc)
{
    fc->rmi_sock = CreateInetServer (fc->config.rmi.proto, 
                                     fc->config.rmi.addr, 
                                     fc->config.rmi.port);
    if (fc->rmi_sock < 0) {
        return -1;
    }

    return 0;
}

int CreateIOServer (jFrameContext *fc)
{
    struct sockaddr_un addr_un;

    unlink (fc->config.general.unix_path);
    fc->io_sock = socket (AF_LOCAL, SOCK_STREAM, 0);
    if (fc->io_sock < 0) {
        return -1;
    }

    addr_un.sun_family = AF_LOCAL;
    strcpy (str_un.sun_path, fc->config.general.unix_path);

    if (bind (fc->io_sock, (struct sockaddr*)&addr_un, sizeof (addr_un)) < 0 ||
        listen (fc->io_sock, 100) < 0) {
        close (fc->io_sock);
        return -1;
    }

    return 0;
}

#define MAX_FD(fd1, fd2) (fd1>fd2?fd1:fd2)

void ServerCore (jFrameContext *fc)
{
    fd_t  serv_fds;
    int ret;
    struct timeval tm_wait = {1, 0};
    int max_fds;

    FD_ZERO (&serv_fds);
    FD_SET (fc->data_sock, &serv_fds);
    FD_SET (fc->rmi_sock, &serv_fds);
    FD_SET (fc->io_sock, &serv_fds);

    max_fds = MAX_FD (fc->io_sock, MAX_FD (fc->data_sock, fc->rmi_sock));
    do {
        ret = select (max_fds+1, &serv_fds, NULL, NULL, &tm_wait);
        if (ret < 0) {

        } else if (ret == 0) {


        } else {


        }
    } while (1);
}

int RunServers (jFrameContext *fc)
{
    if (CreateDataServer (fc) < 0) {

    }

    if (CreateRMIServer (fc) < 0) {

    }

    if (CreateIOServer (fc) < 0) {

    }

    return RunProcInThread (ServerCore, (void*)fc);
}

void JuseRun (jFrameContext *fc)
{
    RunModules (fc);
    RunServers (fc);
}

int JuseInit (jFrameContext *fc, const char *cfg)
{
    if (DC_dict_init (&fc->proc_modules, NULL) < 0) {
        jerr ("DC_dict_init call failed.\n");
        return -1;
    }

    if (LoadConfig (&fc->config, cfg) < 0) {
        return -1;
    }

    if (LoadModules (fc) < 0) {
        return -1;
    }

    return 0;
}


int JuseMain (int argc, const char *argv[], JuseDelegate *delegate)
{
    char *cfg_path;
    jFrameContext *fc;

    if (JuseInit (fc, const char *cfg) < 0) {

    } else {
        JuseRun (fc);
    }

    JuseUninit (fc);

    return 0;
}
