#ifndef _JFRAMEWORK_H
#define _JFRAMEWORK_H

#include "juse.h"
#include "jconfig.h"
#include "jpeer.h"
#include "jmodule.h"
#include "list.h"


typedef struct _jDelegate {

} jDelegate;


typedef struct _jMetaModule {
    union {
        int io_sock;
    };
    unsigned int module_id;
    jModule  *module;
}jMetaModule;


typedef struct _jFrameContext {

#ifdef OS_WINDOWS
#else
    int  data_sock;
    int  rmi_sock;
    int  io_sock;
#endif
    int sig_event;
    pthread_mutex_t ctx_mutex;
    pthread_cond_t  ctx_cond;

    DC_dict_t      proc_modules;
    //jNetbufManager netbuf_manager;
    jConfig        config;
    jModuleManager module_manager;
    jDelegate      *delegate;
    //jPeerManager   peer_manager;
} jFrameContext;


extern jFrameContext *JuseNetwork;
#endif
