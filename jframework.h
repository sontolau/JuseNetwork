#ifndef _JFRAMEWORK_H
#define _JFRAMEWORK_H

#include "juse.h"
#include "jconfig.h"
#include "jpeer.h"
#include "jmodule.h"
#include "list.h"

typedef struct _JuseDelegate {

} JuseDelegate;


typedef struct _jMetaModule {
    char *unix_sock_path;
    jModule  *module;
}jMetaModule;


typedef struct _jFrameContext {

#ifdef OS_WINDOWS
#else
#endif
    JuseDelegate   *delegate;
    jConfig        config;
    DC_dict_t      proc_modules;

    //jNetbufManager netbuf_manager;
    jModuleManager module_manager;
    //jPeerManager   peer_manager;
} jFrameContext;


extern jFrameContext *JuseNetwork;
#endif
