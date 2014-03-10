#ifndef _JFRAMEWORK_H
#define _JFRAMEWORK_H

#include "juse.h"
#include "jconfig.h"
#include "jpeer.h"
#include "jmodule.h"
#include "list.h"


typedef struct _jDelegate {

} jDelegate;

#define MCTL_REG       (1<<1)
#define MCTL_UNREG     (1<<2)
#define MCTL_READ      (1<<3)
#define MCTL_SET_ENABLE  (1<<4)
#define MCTL_SET_DISABLE (1<<5)
#define MCTL_PING        (1<<6)

#define MERR_INVALID_CMD  (1<<0)
#define MERR_OVER_BUFSIZE (1<<1)
#define MERR_NO_MORE_BUF  (1<<2)



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
    DC_list_t      mod_list;
    DC_dict_t      proc_modules;
    jNetbufManager netbuf_manager;
    jConfig        config;
    jModuleManager module_manager;
    DC_list_t      snd_buf_queue;
    DC_list_t      rcv_buf_queue;
    jDelegate      *delegate;
    //jPeerManager   peer_manager;
} jFrameContext;


extern jFrameContext *JuseNetwork;
#endif
