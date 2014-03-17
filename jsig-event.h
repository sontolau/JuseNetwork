#ifndef _JUSE_SIG_EVENT_H
#define _JUSE_SIG_EVENT_H

#include "juse.h"


typedef struct _jSignal {
#ifdef OS_WINDOWS
#else
    pthread_cond_t   sig_cond;
    pthread_mutex_t  sig_mutex;
    //DC_dict_t        sig_vector;
    char             *sig;
#endif    
}jSignal;

extern int SignalInit (jSignal *sig);
extern int SignalWait (jSignal *sig, const char *strsig, int sec);
extern void SignalSend (jSignal *sig, char *signal);
extern void SignalUninit (jSignal *sig);

#endif
