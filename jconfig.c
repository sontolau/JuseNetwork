#ifdef OS_WINDOWS
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <time.h>

#include "jconfig.h"
#include "jinet.h"

#define JCFG_SEC_GENERAL  1
#define JCFG_SEC_MODULE   2
#define JCFG_SEC_RMI      3


static BOOL incorrect_line (char *line)
{
    return FALSE;
}

static int section_name (char *sec)
{
    if (!strcmp (sec, "[general]")) {
        return JCFG_SEC_GENERAL;
    } else if (!strcmp (sec, "[module]")) {
        return JCFG_SEC_MODULE;
    } else if (!strcmp (sec, "[management]")) {
        return JCFG_SEC_RMI;
    }

    return 0;
}

static void cfg_key_value (char *line, char **key, char **value)
{
    char *sym;

    *key   = NULL;
    *value = NULL;

    *key = line;
    sym = strchr (line, (int)'=');
    if (sym) {
        *sym = '\0';
        *value = sym + 1;
    }
}

static void set_cfg_value (jConfig *cfg, int sec, char *key, char *val)
{
    int temp;
    char *strtmp;

    if (!strcmp (key, "addr")) {
        if (sec == JCFG_SEC_GENERAL) {
            cfg->general.addr = inet_addr (val);
        } else if (sec == JCFG_SEC_RMI) {
            cfg->rmi.addr = inet_addr (val);
        }
    } else if (!strcmp (key, "port")) {
        if (sec == JCFG_SEC_GENERAL) {
            cfg->general.port = htons (atoi (val));
        } else if (sec == JCFG_SEC_RMI) {
            cfg->rmi.port = htons (atoi (val));
        }
    } else if (!strcmp (key, "proto")) {
        if (!strcmp (val, "tcp")) {
            temp = INETProtocolTCP;
        } else if (!strcmp (val, "udp")) {
            temp = INETProtocolUDP;
        } else {
            temp = 0;
        }

        if (sec == JCFG_SEC_GENERAL) {
            cfg->general.proto = temp;
        } else if (sec == JCFG_SEC_RMI) {
            cfg->rmi.proto = temp;
        }
    } else if (!strcmp (key, "check-peer-interval") && sec == JCFG_SEC_GENERAL) {
        cfg->general.check_peer_interval = atoi (val);
    } else if (!strcmp (key, "max-clients") && sec == JCFG_SEC_GENERAL) {
        cfg->general.max_clients = atoi (val);
    } else if (!strcmp (key, "max-trans-size") && sec == JCFG_SEC_GENERAL) {
        cfg->general.max_trans_size = (unsigned int)atoi (val);
    } else if (!strcmp (key, "config") && sec == JCFG_SEC_GENERAL) {
        ConfigLoad (cfg, val);
    } else if (!strcmp (key, "num-sock-bufs") && sec == JCFG_SEC_GENERAL) {
        cfg->general.num_sock_bufs = atoi (val);
    } else if (!strcmp (key, "packet-size") && sec == JCFG_SEC_GENERAL) {
        cfg->general.packet_size = (unsigned int)atoi (val);
    } else if (!strcmp (key, "chdir") && sec == JCFG_SEC_GENERAL) {
        cfg->general.chdir = strdup (val);
    } else if (!strcmp (key, "daemon") && sec == JCFG_SEC_GENERAL) {
        cfg->general.daemon = (atoi (val)?TRUE:FALSE);
    } else if (!strcmp (key, "log") && sec == JCFG_SEC_GENERAL) {
        cfg->general.log = strdup (val);
    } else if (!strcmp (key, "moddir") && sec == JCFG_SEC_MODULE) {
        cfg->module.moddir = strdup (val);
    } else if (!strcmp (key, "max-modules") && sec == JCFG_SEC_MODULE) {
        cfg->module.max_modules = atoi (val);
    } else if (!strcmp (key, "load-module") && sec == JCFG_SEC_MODULE) {
        if (cfg->module.modules == NULL) {
            cfg->module.modules = (struct module_config*)calloc (cfg->module.max_modules, sizeof (struct module_config));
            cfg->module.num_modules = 0;
        }

        if (cfg->module.modules && cfg->module.num_modules < cfg->module.max_modules) {
            strtmp = strchr (val, ',');
            if (strtmp) {
                *strtmp = '\0';
                strtmp++;
                temp = atoi (strtmp);
            } else {
                temp  = (int)time (NULL);
            }

            cfg->module.modules[cfg->module.num_modules].mod_id = (unsigned int)temp;
            strncpy (cfg->module.modules[cfg->module.num_modules].mod_name, val, SZMODNAME);
            cfg->module.num_modules++;
        }
    } else if (!strcmp (key, "enable-auth") && sec == JCFG_SEC_RMI) {
        cfg->rmi.enable_auth = (atoi (val)?TRUE:FALSE);
    } else if (!strcmp (key, "auth-script") && sec == JCFG_SEC_RMI) {
        cfg->rmi.auth_script = strdup (val);
    } else if (!strcmp (key, "enable-management") && sec == JCFG_SEC_RMI) {
        cfg->rmi.enable_management = (atoi (val)?TRUE:FALSE);
    }
}

BOOL ConfigLoad (jConfig *cfg, const char *path)
{
    int sec = 0;
    FILE *fp;
    char line[255];
    char *key, *val;

    fp = fopen (path, "r");
    if (fp == NULL) {
        return FALSE;
    }

    while (!feof (fp)) {
        memset (line, '\0', sizeof (line));
        fgets (line, sizeof (line) - 1, fp);
        line[strlen(line) - 1] = '\0';
   
        if (incorrect_line (line)) {
            continue;
        }    
 
        if (!(sec = section_name (line))) {
            continue;
        } else {
            cfg_key_value (line, &key, &val);
            if (key && val) {
                set_cfg_value (cfg, sec, key, val);
            }
        }
    }

    fclose (fp);
    return TRUE;
}

void ConfigRelease (jConfig *cfg)
{
    free (cfg->general.log);
    free (cfg->general.chdir);
    free (cfg->module.moddir);
    free (cfg->module.modules);
    free (cfg->rmi.auth_script);
}
