#include "jframework.h"

///////////////////////////////////
//  Module operations
///////////////////////////////////


jMetaModule *FindModuleWithID (jFrameContext *fc, unsigned int mid)
{
    int i;
    char strid[10];
    static jMetaModule *module = NULL;
   
    if (module && module->module_id == mid) {
        return module;
    }

    if (pthread_mutex_lock (&fc->ctx_mutex) == 0) {
#if 0
        for (i=0; i<fc->modules.count; i++) {
            module = DC_list_get_object_at_index (&fc->modules, i)
            if (module && module->module_id == mid) {
                break;
            }
        }
#endif
        sprintf (strid, "%lu", mid);
        module = DC_dict_get_object_with_key (&fc->engaged_modules,
                                              (char*)strid);
        pthread_mutex_unlock (&fc->ctx_mutex);
    }

    return module;
}

void *FindObjectInLists (jFrameContext *fc, int type_idx, int index)
{
    int i;
    void *obj = NULL;

    if (pthread_mutex_lock (&fc->ctx_mutex) == 0) {
        obj = DC_list_object_at_index (&fc->core_lists[type_idx],index);
        pthread_mutex_unlock (&fc->ctx_mutex);
    }
    return obj;
}


void RemoveProcModuleWithID (jFrameContext *fc, unsigned int mid)
{
    char strid[10] = {0};
    static jMetaModule *module == NULL;

    if (pthread_mutex_lock (&fc->ctx_mutex) == 0) {
        if (module && module->module_id == mid) {
            goto rm_module;
        }
        for (i=0; i<fc->modules.count; i++) {
            module = DC_list_get_object_at_index (&fc->modules, i);
            if (module && module->module_id == mid) {
rm_module:
                DC_list_remove_object (&fc->modules, (void*)module);
                free (module);
                break;
            }
        }

        pthread_mutex_unlock (&fc->ctx_mutex);
    }

}

jMetaModule *AllocNewModule (jFrameContext *fc, unsigned int mid)
{
    char strid[10];
    jMetaModule *module = NULL;
   
    module = (jMetaModule*)calloc (1, sizeof (jMetaModule));
    if (module && pthread_mutex_lock (&fc->ctx_mutex) == 0) {
        sprintf (strid, "%lu", mid);
        module->module_id = mid;
        DC_dict_add_object_with_key (&fc->engaged_modules, (char*)strid, (void*)module);
        pthread_mutex_unlock (&fc->ctx_mutex);
    }

    return module;
}

int DisableModule (jMetaModule *mod)
{
    int cmd[2] = {MCTL_DISABLE, 0};

    if (send (mod->sock_fd, &cmd[0], sizeof (int), 0) >= sizeof (int)
        && recv (mod->sock_fd, cmd, sizeof (cmd), 0) >= sizeof (cmd)
        && cmd[0] == MCTL_DISABLE
        && cmd[1] == 0) {
        return 0;
    }

    return -1;
}


int EnableModule (jMetaModule *mod)
{
    int cmd[2] = {MCTL_ENABLE, 0};

    if (send (mod->sock_fd, &cmd[0], sizeof (int), 0) >= sizeof (int)
        && recv (mod->sock_fd, cmd, sizeof (cmd), 0) >= sizeof (cmd)
        && cmd[0] == MCTL_ENABLE
        && cmd[1] == 0) {
        return 0;
    }

    return -1;
}

int QueryModule (jMetaModule *mod, unsigned int *flags)
{
    int cmd[3] = {MCTL_QUERY, 0, 0};

    if (send (mod->sock_fd, &cmd[0], sizeof (int), 0) >= sizeof (int)
        && recv (mod->sock_fd, cmd, sizeof (cmd), 0) >= sizeof (cmd)
        && cmd[0] == MCTL_QUERY
        && cmd[1] == 0) {
        *flags = cmd[2];
        return 0;
    }

    return -1;

}

int WriteModule (jMetaModule *mod, jPeer *peer)
{
    unsigned int cmd_arg[3] = {MCTL_WRITE, peer->pid, peer->net_buf.length};

    if (send (mod->sock_fd, cmd_arg, sizeof (cmd_arg), 0) >= sizeof (cmd_arg)
        && send (mod->sock_fd, peer->net_buf.data, peer->net_buf.length, 0) 
                                                    >= peer->net_buf.length
        && recv (mod->sock_fd, cmd_arg, 2*sizeof (int), 0) >= 2*sizeof (int)) {
        return 0;
    }

    return -1;
}

int PingModule (jMetaModule *mod)
{
    unsigned int cmd[] = {MCTL_PING, 0};

    if (send (mod->sock_fd, &cmd[0], sizeof (int), 0) >= sizeof (int)
        && recv (mod->sock_fd, &cmd[0], sizeof (cmd), 0) >= sizeof (cmd)
        && cmd[0] == MCTL_PING
        && cmd[1] == 0) {
        return 0;
    }

    return -1;
}

int RegisterModule (jMetaModule *mod)
{
    unsigned int cmd_arg[3] = {MCTL_REGISTER, 0, 0};
    if (send (mod->sock_fd, &cmd_arg[0], sizeof (int), 0) >= sizeof (int)
        && recv (mod->sock_fd, &cmd_arg[0], sizeof (cmd_arg), 0) >= sizeof (cmd_arg)
        && cmd_arg[0] == MCTL_REGISTER
        && cmd_arg[1] == 0) {
        mod->module_id = cmd_arg[2];
        return 0;
    }
    return -1;
}

int RunProcInCritical (jFrameContext *fc, void (*code)(void*), void *data)
{
    if (pthread_mutex_lock (&fc->ctx_mutx) == 0) {
        code (data);
        pthread_mutex_unlock (&fc->ctx_mutex);
        return 0;
    }

    return -1;
}

int RunProcInThread (void (*proc)(void*), void *data)
{
    pthread_t  pt;

    if (pthread_create (&pt, NULL, &proc, data) < 0) {
        return -1;
    }

    return 0;
}

void SetDefaultConfig (jConfig *cfg)
{
    cfg->general.proto = PROTO_UDP;
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
    cfg->rmi.proto = PROTO_TCP;
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

extern JuseUninit (jFrameContext *fc);

int RunModuleInProcess (jFrameContext *fc, int index)
{
    
    JuseUninit (fc);
}

void RunModules (jFrameContext *fc)
{
    int i;
    char strmid[50];
    jMetaModule *mmod;
    pid_t pid;

    for (i=0; i<fc->config.module.num_modules; i++) {
        pid = fork ();
        if (pid < 0) {
            jerr ("fork call failed.\n");
        } else if (pid == 0) {
            exit (RunModuleInProcess (fc, i));
        }
    }
    
}

int CreateInetServer (int proto, unsigned int addr, unsigned short port)
{
    int sock;
    struct sockaddr_in addr;
    int flag = 1;
    unsigned int szsockbuf = 0xFFFF;

    if (proto == PROTO_TCP) {
        sock = socket (AF_INET, SOCK_STREAM, 0);
    } else if (proto == PROTO_UDP) {
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

    if (proto == PROTO_TCP) {
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

jPeer *GetPeerFromQueue (jFrameContext *fc)
{
    jPeer *peer = NULL;

    if (pthread_mutex_lock (&fc->ctx_mutex) == 0) {
        peer = DC_list_get_object_at_index (&fc->peer_queue, 0);
        DC_list_remove_object_at_index (&fc->peer_queue, 0);
        pthread_mutex_unlock (&fc->ctx_mutex);
    }

    return peer;
}

int WaitForEvent (jFrameContext *fc, int ev)
{
    int ret;
    int sig;

    do {
        pthread_mutex_lock (&fc->ctx_mutex);
        ret = pthread_cond_wait (&fc->ctx_cond, &fc->ctx_mutex);
        if (ret == 0) {
            sig = fc->event;
        }
        pthread_mutex_unlock (&fc->ctx_mutex);
    } while (sig != ev && sig != jEventQuit);

    return sig;
}

int SendEvent (jFrameContext *fc, int ev)
{
    if (pthread_mutex_trylock (&fc->ctx_mutex) == 0) {
        fc->event = ev;
        pthread_mutex_unlock (&fc->ctx_mutex);
        return 0;
    }
    return -1;
}

void ReadWriteProc (void *data)
{
    jFrameContext *fc = (jFrameContext*)data;
    int ret;
    jPeer *peer;

    do {
        ret = WaitForEvent (fc, jEventReadWrite);
        if (ret < 0) {

        } else if (ret == jEventReadWrite) {
            while ((peer = GetPeerFromQueue (fc))) {
                if (peer->netbuf->intval == EV_READ) {

                } else if (peer->netbuf->intval == EV_WRITE) {

                }
            }
        } else if (ret == jEventQuit) {
        }
    }
}

int RunServers (jFrameContext *fc)
{
    RunProcInThread (ReadWriteProc, fc);

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

void JuseUninit (jFrameContext *fc)
{
    int i;

    pthread_cond_destroy (&fc->ctx_cond);
    pthread_mutex_destroy (&fc->ctx_mutex);
    ModuleManagerUninit (&fc->module_manager);
    NetbufManagerUninit (&fc->netbuf_manager);
    PeerManagerUninit (&fc->peer_manager);

    for (i=0; i<sizeof (fc->core_lists)/sizeof (DC_list_t); i++) {
        DC_list_destroy (&fc->core_lists[i]);
    }

    DC_dict_destroy (&fc->engaged_modules);
    //DC_list_destroy (&fc->peer_list);
    //DC_list_destroy (&fc->msock_list);
}

int JuseInit (jFrameContext *fc, const char *cfg)
{
    int i;

    if (LoadConfig (&fc->config, cfg) < 0) {
        return -1;
    }

    for (i=0; i<sizeof (fc->core_lists)/sizeof (DC_list_t); i++) {
        DC_list_init (&fc->core_lists[i], NULL);
    }

    DC_dict_init (&fc->engaged_modules, NULL);
    if (!NetbufManagerInit (&fc->netbuf_manager, fc->config.general.max_trans_size)) {

    }

    if (!ModuleManagerInit (&fc->module_manager)) {

    }

    if (!PeerManagerInit (&fc->peer_manager, 1000)) {

    }

    fc->err_payloads = ErrorPayloads ();

    if (phread_mutex_init (&fc->ctx_mutex, NULL) < 0) {
        return -1;
    }

    if (pthread_cond_init (&fc->ctx_cond, NULL) < 0) {
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
