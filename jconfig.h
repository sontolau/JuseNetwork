#ifndef _JCONFIG_H
#define _JCONFIG_H

#include "juse.h"
#include "jinet.h"

#define SZMODNAME  50

struct module_config {
    unsigned int mod_id;
    char mod_name[SZMODNAME + 1];
};

typedef struct _jConfig {
    struct {
        int proto;
        unsigned int addr;
        unsigned short port;
        BOOL daemon;
        char *chdir;
        char *log;
        char *unix_path;
        int check_peer_interval;
        int max_clients;
        int num_sock_bufs;
        unsigned int max_trans_size;
        unsigned int packet_size;
    } general;

    struct {
        char *moddir;
        int conn-retry;
        int max_modules;
        int num_modules;
        struct module_config *modules;
    } module;

    struct {
        BOOL enable_management;
        int proto;
        BOOL enable_auth;
        char *auth_script;
        unsigned int addr;
        unsigned short port;
    } rmi;
} jConfig;


extern BOOL ConfigLoad (jConfig *cfg, const char *path);

#endif
