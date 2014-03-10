<F5>ty#include "jframework.h"

int ServerCore (int *fd_list, int num, void (*core)(void*), void *data)
{
    fd_t fds;
    int ret;
    struct timeval tm_wait = {0, 0};
    int i, maxfd=0;

    FD_ZERO (&fds);
    for (i=0; i<num; i++) {
        FD_SET (fd_list[i], &fds);
        maxfd = (maxfd > fd_list[i]?maxfd:fd_list[i]);
    }

    while (1) {
        tm_wait.tv_sec = 1;
        ret = select (maxfd + 1, &fds, NULL, NULL, &tm_wait);
        if (ret < 0) {

        } else if (ret == 0) {

        
    }
}

void DataServerCore (jFrameContext *fc)
{
    fd_t fds;
    int ret;
    struct timeval tm_wait = {1, 0};

    FD_ZERO (&fds);
    FD_SET (fc->data_sock, &fds);
    
    while (1) {
        tm_wait.tv_sec = 1;
        ret = select (fc->data_sock + 1, &fds, NULL, NULL, &tm_wait);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
        } else if (ret == 0) {

        } else {

        }
    }
}



void RMIServerCore (jFrameContext *fc)
{
    fd_t fds;
    int ret;
    struct timeval tm_wait = {1, 0};

    FD_ZERO (&fds);
    FD_SET (fc->rmi_sock, &fds);
    while (1) {
        tm_wait.
    }
}


void IOServerCore (jFrameContext *fc)
{
}
