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
#define MCTL_WRITE      (1<<3)
#define MCTL_ENABLE  (1<<4)
#define MCTL_DISABLE (1<<5)
#define MCTL_QUERY (1<<6)
#define MCTL_PING        (1<<7)

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

enum {
    jEventReadWrite = 1,
    jEventQuit,
};



typedef struct _jServer {
    
};

typedef struct _jFrameContext {

#ifdef OS_WINDOWS
#else
    int  data_sock;
    int  rmi_sock;
    int  io_sock;
#endif
    int event;
    pthread_mutex_t ctx_mutex;
    pthread_cond_t  ctx_cond;
    //DC_list_t       msock_list;
#define I_SNDBUF   0
#define I_RCVBUF   1
#define I_MSOCK  2
#define I_PEER    3
    DC_list_t      core_lists[5];
    DC_dict_t      engaged_modules;
    //DC_list_t      modules;
    jNetbufManager netbuf_manager;
    jConfig        config;
    jModuleManager module_manager;
    jDelegate      *delegate;
    jPeerManager   peer_manager;
    jPayload       **err_payloads;
} jFrameContext;


extern jFrameContext *JuseNetwork;
#endif
