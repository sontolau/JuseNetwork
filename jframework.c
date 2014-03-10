#include "jframework.h"


int RunProcInThread (void (*proc)(void*), void *data)
{
    pthread_t  pt;

    if (pthread_create (&pt, NULL, &proc, data) < 0) {
        return -1;
    }

    return 0;
}

jMetaModule *LoadMouleInProcess (jFrameContext *jc, unsigned int midx)
{
    char modpath[255];
    jMetaModule *module = NULL;

    snprintf (modpath, sizeof (modpath)-1, "%s/%s/%s",
               fc->config.general.chdir,
               fc->config.module.moddir,
               fc->config.module.modules[midx].mod_name);
     
    if (!fc->module_manager.load_module (&fc->module_manager,
                                         fc->config.module.modules[midx].mod_id,
                                         modpath)) {
            return NULL;
    } else {
            module = (jMetaModule*)calloc (1, sizeof (jMetaModule));
            if (module) {
                module->io_sock = 0;
                module->module_id = fc->config.module.modules[midx].mod_id;
                module->module = fc->module_manager.get_module (&fc->module_manager,
                                                                fc->config.module.modules[midx].mod_id);
            } else {
                fc->module_manager.unload_module (&fc->module_manager,
                                                  fc->config.module.modules[midx].mod_id);
            }

    }

}

void ModuleCoreProc (jFrameContext *fc, unsigned int mid)
{
     if (!MoudleManagerInit (&fc->module_manager)) {

     }

     LoadModuleInProcess (fc, mid);
}

int LoadModules (jFrameContext *fc)
{
    int i;
    unsigned int mid;
    pid_t pid;

    for (i=0; i<fc->config.module.num_modules; i++) {
        mid = fc->config.module.modules[i].mod_id;

        pid = fork ();
        if (pid < 0) {

        } else if (pid == 0) {
            ModuleCoreProc (fc, mid);
        } else {

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
    cfg->general.daemon = FALSE;
    cfg->general.check_peer_interval = 1;
    cfg->general.max_clients = 1000;
    cfg->general.max_trans_size = 1400;
    cfg->general.udp_packet_size = 1400;

    cfg->module.moddir = "modules";
    cfg->module.conn-retry = 3;
    cfg->module.max_modules = 10;
    cfg->module.num_modules = 0;
    cfg->module.modules = NULL;

    cfg->rmi.enable_management = FALSE;
    cfg->rmi.proto = INETProtocolTCP;
    cfg->rmi.enable_auth = FALSE;
    cfg->rmi.auth_script = NULL;
    cfg->rmi.addr        = INADDR_ANY;
    cfg->rmi.port        = 4041;
}

int CheckConfig (jConfig *cfg)
{
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

extern void DataServerCore (jFrameContext *fc);
extern void RMIServerCore (jFrameContext *fc);
extern void IOServerCore (jFrameContext *fc);

int CreateDataServer (jFrameContext *fc)
{
    fc->data_sock = CreateInetServer (fc->config.general.proto, 
                                      fc->config.general.addr,
                                      fc->config.general.port);
    if (fc->data_sock < 0) {
        return -1;
    }

    RunProcInThread (DataServerCore, fc);
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

    RunProcInThread (RMIServerCor, fc);
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

    RunProcInThread (IOServerCore, fc);
    return 0;
}

#define MAX_FD(fd1, fd2) (fd1>fd2?fd1:fd2)

int JuseCore (jFrameContext *fc)
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
    if (RunDataServer (fc) < 0) {

    }

    if (fc->config.rmi.enable_management 
        && RunRMIServer (fc) < 0) {
        return -1;
    }

    if (RunIOServer (fc) < 0) {

    }

    return JuseCore (fc);
}

void SetJuseSystem (jFrameContext *fc)
{
    if (fc->config.general.daemon) {
        daemon (0, 0);
    }

     if (fc->config.general.chdir) {
        chdir (fc->config.general.chdir);
    }

}

void JuseRun (jFrameContext *fc)
{
    SetJuseSystem (fc);

    RunModules (fc);

    RunServers (fc);
}

int JuseInit (jFrameContext *fc, const char *cfg)
{
    if (DC_dict_init (&fc->proc_modules, NULL) < 0) {
        jerr ("DC_dict_init call failed.\n");
        return -1;
    }

    if (phread_mutex_init (&fc->ctx_mutex, NULL) < 0) {
        return -1;
    }

    if (pthread_cond_init (&fc->ctx_cond, NULL) < 0) {
        return -1;
    }

    if (LoadConfig (&fc->config, cfg) < 0) {
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
