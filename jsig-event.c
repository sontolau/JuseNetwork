#include "jsig-event.h"


int SignalInit (jSignal *sig)
{
/*
    if (DC_dict_init (&sig->sig_vector, NULL) < 0) {
        return -1;
    }
*/
#ifdef OS_WINDOWS
#else
    pthread_mutex_init (&sig->sig_mutex, NULL);
    pthread_cond_init (&sig->sig_cond, NULL);
#endif
    sig->sig = NULL;
    return 0;
}
/*
int SignalSet (jSignal *sig, char *sigev, void (*sig_handler)(void*), void *data)
{
    struct jHandler *h;

    h = (struct jHandler*)calloc (1, sizeof (struct jHandler));
    if (h == NULL)
        return -1;
    }

    pthread_mutex_lock (&sig->sig_mutex);
    h->sig_handler = sig_handler;
    h->data        = data;
    DC_dict_add_object_with_key (&sig->sig_vector, sigev, (void*)h);
    pthread_mutex_unlock (&sig->sig_mutex);

    return 0;
}
*/
int SignalWait (jSignal *sig, const char *sigwait, int sec)
{
    //jHandler *h;
    struct timespec ts = {sec, 0};
    int ret;
    do {
        if (!(pthread_mutex_lock (&sig->sig_mutex);
              ||(ret = pthread_cond_timedwait (&sig->sig_cond, &sig->sig_mutex, &ts)))) {
            if (ret == 0) {
                ret = strcmp (sig->sig, sigwait);
                pthread_mutex_unlock (&sig->sig_mutex);
                if (ret == 0) return 0;
            } else if (ret == ETIMEDOUT) {
                return 0;
            }
        } else {
            return -1;
        }
    } while (1);

    return -1;
}

void SignalSend (jSignal *sig, char *sigev)
{
    pthread_mutex_lock (&sig->sig_mutex);
    sig->sig = sigev;
    pthread_mutex_unlock (&sig->sig_mutex);

    pthread_cond_boardcast (&sig->sig_cond);
}



void SignalUninit (jSignal *sig)
{
    pthread_mutex_destroy (&sig->sig_mutex);
    pthread_cond_destroy (&sig->sig_cond);
}
