#include "jserver.h"
#include "jheader.h"

#define SIG_CHECK_PEER "signal_check_peer"

//创建服务器。
int      ServerCreate (jServer *serv,
                       int proto, 
                       const char *addr, 
                       unsigned short port)
{
    int flags = 1;
    struct sockaddr *skaddr;
    struct sockaddr_in inaddr;
    struct sockaddr_un unaddr;

    if (pthread_rwlock_init (&serv->rwlock, NULL) < 0) {
        return -1;
    }

    serv->inet_info.proto = proto;
    serv->inet_info.port  = port;

    if (port > 0) {
        skaddr = (struct sockaddr*)&inaddr;

        ((struct sockaddr_in*)skaddr)->sin_family = AF_INET;
        ((struct sockaddr_in*)skaddr)->sin_port   = htons (port);
        ((struct sockaddr_in*)skaddr)->sin_addr.s_addr = inet_addr (addr);
        serv->inet_info.addr = inet_addr (addr);

        if (proto == PROTO_TCP) {
            serv->inet_info.sock_fd = socket (AF_INET, SOCK_STREAM, 0);
        } else if (proto == PROTO_UDP) {
            serv->inet_info.sock_fd = socket (AF_INET, SOCK_DGRAM, 0);
        }
    } else {
        skaddr = (struct sockaddr*)&unaddr;

        ((struct sockaddr_un*)skaddr)->sun_family = AF_LOCAL;
        strcpy (((struct sockaddr_un*)skaddr)->sun_path, addr);
        serv->inet_info.unix_path = strdup (addr);

        if (proto == PROTO_TCP) {
            serv->inet_info.sock_fd = socket (AF_LOCAL, SOCK_STREAM, 0);
        } else if (proto == PROTO_UDP) {
            serv->inet_info.sock_fd = socket (AF_LOCAL, SOCK_DGRAM, 0);
        }
    }

    if (serv->inet_info.sock_fd < 0) {
        return -1;
    }

    setsockopt (serv->inet_info.sock_fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof (int));
    //设置非阻塞模式。
    fcntl (serv->inet_info.sock_fd, F_GETFL, &flags);
    flags |= O_NONBLOCK;
    fcntl (serv->inet_info.sock_fd, F_SETFL, flags);

    if (bind (serv->inet_info.sock_fd, skaddr, sizeof (struct sockaddr)) < 0) {
        close (serv->inet_info.sock_fd);
        return -1;
    }

    if (proto == PROTO_TCP) {
        listen (serv->inet_info.sock_fd, 100);
    }

    return 0;
}


int DoRequest (jPeer *peer, jServer *serv)
{
    jTransHeader *hdr;
    struct sockaddr skaddr;
    socklen_t sklen = sizeof (skaddr);
    int ret;
    int status = 0;
    jError  error = {EOK, EOK};
    char strid[10] = {0};
    static jNetbuf  *netbuf;
    unsigned int szbuf;
    jPeer  *newpeer = NULL;
    //申请一块用以接受数据的缓冲区。
    szbuf = serv->max_trans + SZTRANSHDR;
    netbuf = serv->nb_manager.alloc_buffer (&serv->nb_manager, szbuf);
    if (netbuf == NULL) {
        return 0;
    }

    //接受数据。
    hdr = (jTransHeader*)netbuf->data;
    if (peer->inet_info.proto == PROTO_TCP) {
        ret = recv (peer->inet_info.sock_fd, netbuf->data, szbuf, 0);
        getpeername (peer->inet_info.sock_fd, &skaddr, &sklen);
    } else if (peer->inet_info.proto == PROTO_UDP) {
        ret = recvfrom (peer->inet_info.sock_fd, netbuf->data, szbuf, 0, &skaddr, &sklen);
    }
   
    //测试网路状态。 
    if (ret <= 0) {
        if (errno == EAGAIN) {
            return 0;
        }
        return -1;
    }

    netbuf->length = (unsigned int)ret;
    sprintf (strid, "%u", hdr->PID);
    switch (hdr->command) {
        //处理JCOM_CONN命令，该命令用于连接到本服务区。
        case JCOM_CONN:
            if (JUSE_REPLY(*hdr)) {
                
            }

            if (peer->inet_info.proto == PROTO_UDP) {
                //分配一个新的节点为新连接的客户。
                peer = serv->peer_manager.new_peer (&serv->peer_manager);
                if (newpeer == NULL) {
                    return 0;
                }
                peer->inet_info.addr = serv->inet_info.addr;
                peer->inet_info.port = serv->inet_info.port;
                peer->inet_info.sock_fd = serv->inet_info.sock_fd;
            } else if (peer->inet_info.proto == PROTO_TCP) {
                peer->inet_info.addr = ((struct sockaddr_in*)&skaddr)->sin_addr.s_addr;
                peer->inet_info.port = ((struct sockaddr_in*)&skaddr)->sin_port;
            }
            peer->pid = PID();
            if (serv->connect) {
                status = serv->connect (serv, peer, serv->private_data, &error);
            }

            if (status == 0) {
                peer->conn_flag = TRUE;
                DC_dict_add_object_with_key (&serv->conn_peers, (char*)strid, (void*)peer);
                if (peer->inet_info.proto == PROTO_UDP) {
                    DC_list_add_object (&serv->sock_peers, (void*)peer);
                }
            } else if (peer->inet_info.proto == PROTO_UDP){
                serv->peer_manager.release_peer (&serv->peer_manager, peer);
            }
            break;
        case JCOM_DISCONN:
            if (JUSE_REPLY (*hdr)) {

            }

            peer = DC_dict_get_object_with_key (&serv->conn_peers, (char*)strid);
            if (peer) {
                if (serv->disconnect) {
                    serv->disconnect (serv, peer, serv->private_data, &error);
                }
                DC_dict_remove_object_with_key (&serv->conn_peers, (char*)strid);
            }
            break;
        case JCOM_PING:
            if (JUSE_REPLY (*hdr)) {
                
            }
            break;
        case JCOM_TRANS:
            if (JUSE_REPLY (*hdr)) {

            }
            peer = DC_dict_get_object_with_key (&serv->conn_peers, (char*)strid);
            if (peer) {
                if (serv->transaction) {
                    status = serv->transaction (serv, peer, serv->private_data, &error);
                }
            }
            break;
        default:
            break;
    }

    if (status) {
#if 0
        hdr->error.eclass = error.eclass;
        hdr->error.ecode  = error.ecode;
        hdr->length       = 0;

        if (peer->net_info.proto == PROTO_TCP) {
            if (send (peer->inet_info.sock_fd, hdr, SZJUSEHDR, 0) < SZJUSEHDR) {

            }
        } else if (peer->net_info.proto == PROTO_UDP) {
            if (sendto (peer->inet_info.sock_fd, hdr, SZJUSEHDR, 0, skaddr, sizeof (skaddr)) < SZJUSEHDR) {

            }
        }
#endif
    }
    return 0;
}


int ServProc (jServer *serv, jPeer *peer)
{
    jPeer *newpeer;
    int csock;

        if (peer->inet_info.sock_fd == serv->inet_info.sock_fd) {
            if (serv->inet_info.proto == PROTO_TCP) {
                csock = accept (serv->inet_info.sock_fd, NULL, NULL);
                if (csock > 0) {
                    newpeer = serv->peer_manager.new_peer (&serv->peer_manager);
                    if (newpeer) {
                        memset (newpeer, '\0', sizeof (jPeer));
                        newpeer->inet_info.sock_fd = csock;
                        newpeer->inet_info.proto   = peer->inet_info.proto;
                        newpeer->conn_time         = serv->ping_count;
                        DC_list_add_object (&serv->sock_peers, (void*)newpeer);
                        return csock;
                    } else {
                        close (csock);
                    }
                }
            } else if (serv->inet_info.proto == PROTO_UDP) {
                DoRequest (peer, serv);
            }
        } else {
            DoRequest (peer, serv);
        }
    return 0;
}

int DoIORequest (jServer *serv,int num, void *data)
{
    void *saveptr = NULL;
    jPeer *peer;
    int max_fd = 0;
    int newfd;

    do {
        if (pthread_rwlock_rdlock (&serv->rwlock) < 0) {
            return -1;
        }

        peer = DC_list_next_object (&serv->sock_peers, &saveptr);

        if (peer) {
            if (FD_ISSET (peer->inet_info.sock_fd, (fd_set*)data)) {
                max_fd = (max_fd > peer->inet_info.sock_fd?max_fd:peer->inet_info.sock_fd);
                newfd = ServProc (serv, peer);
                if (newfd > 0) {
                    FD_SET (newfd, (fd_set*)data);
                    max_fd = (max_fd > newfd?max_fd:newfd);
                }
            }
        }
        
        pthread_rwlock_unlock (&serv->rwlock);
    } while (peer && num > 0);

    return max_fd;
}

void *JuseServer (void *data)
{
    jServer *serv = (jServer*)data;
    int max_fd;
    int ret;
    struct timeval timew;
    fd_set fds;
    jPeer *peer;

    FD_ZERO (&fds);
    FD_SET (serv->inet_info.sock_fd, &fds);
    max_fd = serv->inet_info.sock_fd;

    //为服务器建立节点。
    peer = serv->peer_manager.new_peer (&serv->peer_manager);
    memset (peer, '\0', sizeof (jPeer));
    peer->inet_info.sock_fd = serv->inet_info.sock_fd;
    peer->inet_info.proto   = serv->inet_info.proto;
    peer->inet_info.port    = serv->inet_info.port;
    peer->pid               = 0;
    DC_list_add_object (&serv->sock_peers, (void*)peer);

     do {
        timew.tv_sec = serv->time_wait;
        timew.tv_usec = 0;

        ret = select (max_fd+1, &fds, NULL, NULL, serv->time_wait?&timew:NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            return (void*)-1;
        } else if (ret == 0) {
            serv->ping_count++;
            if (serv->timeout) {
                serv->timeout (serv, serv->private_data);
            }
        } else {
            max_fd = DoIORequest (serv, ret, (void*)&fds);
        }
    } while (!serv->quit_flag);

    return NULL;
}

void *PeerKeeper (void *data)
{
    int ret;
    jServer *serv = (jServer*)data;

    do {
        ret = SignalWait (&serv->signal, SIG_CHECK_PEER, 1);
        if (ret < 0) {
            break;
        } else if (ret == 0) {

        } else {

        }
    } while (1);

    return NULL;   
}

int RunPeerKeeper (jServer *serv)
{
    pthread_t pt;

    if (pthread_create (&pt, NULL, PeerKeeper, (void*)serv) < 0) {
        return -1;
    }

    pthread_detach (pt);
    return 0;
}

int ServerRun (jServer *serv, int thread)
{
    if (SignalInit (&serv->signal) < 0) {
        return -1;
    }

    if (!PeerManagerInit (&serv->peer_manager, serv->max_peers + 1)) {
        return -1;
    }

    if (!NetbufManagerInit (&serv->nb_manager, 
                            serv->max_trans + SZTRANSHDR + 10, 
                            serv->max_peers + 1)) {
        return -1;
    }

    if (RunPeerKeeper (serv) < 0) {
        return -1;
    }

    if (thread) {
        if (pthread_create (&serv->run_thread, NULL, JuseServer, (void*)serv) < 0) {
            return -1;
        }
    } else {
        serv->run_thread = pthread_self ();
        JuseServer (serv);
    }

    return 0;
}

void ClosePeers (jServer *serv)
{
    void *dataptr = NULL;
    jPeer *peer;
    jError error = {EOK, EOK};

    while ((peer = DC_list_next_object (&serv->sock_peers, &dataptr))) {
        if (peer->conn_flag) {
            if (serv->disconnect) {
                serv->disconnect (serv, peer, serv->private_data, &error);
            }
        }

        if (peer->inet_info.sock_fd > 0) {
            close (peer->inet_info.sock_fd);
        }
    }
}

void ServerQuit (jServer *serv, int wait)
{
    pthread_rwlock_wrlock (&serv->rwlock);
    serv->quit_flag = 1;
    pthread_rwlock_unlock (&serv->rwlock);

    if (wait) {
        pthread_join (serv->run_thread, NULL);
    }
}
void ServerClose (jServer *serv)
{
    pthread_rwlock_wrlock (&serv->rwlock);
    ClosePeers (serv);
    PeerManagerUninit (&serv->peer_manager);
    NetbufManagerUninit (&serv->nb_manager);
    DC_list_destroy (&serv->sock_peers);
    DC_dict_destroy (&serv->conn_peers);
    pthread_rwlock_destroy (&serv->rwlock);
}
