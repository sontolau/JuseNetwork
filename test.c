#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>


int flag = 1;
int number = 1;

void sig_func (int sig)
{
    switch (sig) {
        case SIGINT:
        case SIGTERM:
            flag = 0;
            break;
        case SIGUSR1:
            printf ("SIGUSR1\n");
            break;
    }
}
void *thread_func (void *arg)
{
    fd_set fds;
    int ret;
    FD_ZERO (&fds);

    while (flag) {
        if (ret < 0) {
            if (errno == EINTR) {
                printf ("number is : %d [%u]\n", number, pthread_self ());
                fprintf (stderr, "thread [%u] is waked up", pthread_self());
                continue;
            } else {
                break;
            }
        }
    }

    fprintf (stderr, "thread exited[%u]\n", pthread_self());
    return NULL;
}

int main ()
{
    pthread_t pt;

    int i;

    signal (SIGINT, sig_func);
    signal (SIGUSR1, sig_func);
    signal (SIGTERM, sig_func);

    for (i=0; i<10; i++) {
        pthread_create (&pt, NULL, thread_func, NULL);
    }

    fd_set fds;
    FD_ZERO (&fds);
   
    while (flag) {
        select (0, &fds ,NULL, NULL, NULL);
        printf ("NI hao\n");
    }
    
    return 0;
}
