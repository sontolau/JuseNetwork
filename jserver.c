int      ServerCreate (jServer *serv,
                       int proto, 
                       const char *addr, 
                       unsigned short port)
{
    int flags = 1;
    struct sockaddr *skaddr;
    struct sockaddr_in inaddr;
    struct sockaddr_un unaddr;

    //DC_list_init (&serv->sock_peers, NULL);
    //DC_list_init (&serv->peers, NULL);
    //NetbufManagerInit (&serv->mem_netbuf, maxbufs);
    //PeerManagerInit (&serv->mem_peers, maxpeers);

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
        ServerClose (serv);
        return NULL;
    }

    setsockopt (serv->inet_info.sock_fd, SOL_SOCKET, SO_REUSEADDR, flags, sizeof (int));
    fcntl (serv->inet_info.sock_fd, FD_GETFL, &flags);
    flags |= O_NONBLOCK;
    fcntl (serv->inet_info.sock_fd, FD_SETFL, flags);

    if (bind (serv->inet_info.sock_fd, skaddr, sizeof (struct sockaddr)) < 0) {
        ServerClose (serv);
        return NULL;
    }

    if (proto == PROTO_TCP) {
        listen (serv->inet_info.sock_fd, 100);
    }
    return serv;
}


int ServerSetOptions (jServer *serv, 
                      unsigned int max_trans, 
                      int ping_interval)
{
    serv->ping_interval = ping_interval;
    serv->max_trans     = max_trans;
     
}
                      
int DoRequest (jPeer *peer, jServer *serv, unsigned int max_trans)
{
    jTransHeader *hdr;
    struct sockaddr skaddr;
    socklen_t sklen = sizeof (skaddr);
    int ret;
    jPeer *peer = NULL;
    char strid[10] = {0};
    static jNetbuf  *netbuf;
    unsigned int szbuf;

    //申请一块用以接受数据的缓冲区。
    szbuf = max_trans + SZTRANSHDR;
    netbuf = serv->mem_netbuf.alloc_buffer (&serv->mem_netbuf, SZTRANSHDR);
    if (netbuf == NULL) {
        return 0;
    }

    //接受数据。
    hdr = (jTransHeader*)netbuf->data;
    if (peer->inet_info.proto == PROTO_TCP) {
        ret = recv (peer->inet_info.sock_fd, netbuf->data, szbuf, 0);
        getpeername (sock, &skaddr, &sklen);
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
    switch (hdr->command) {
        case JCOM_CONN:
            if (proto == PROTO_UDP) {
                peer = serv->mem_peers.new_peer (&serv->mem_peers);
                if (peer) {
                    peer->inet_info.addr = serv->inet_info.addr;
                    peer->inet_info.port = serv->inet_info.port;
                    peer->inet_info.sock_fd = serv->inet_info.sock_fd;
                }
            } else if (proto === PROTO_TCP) {
                peer->inet_info.addr = ((struct sockaddr_in*)&skaddr)->sin_addr.s_addr;
                peer->inet_info.port = ((struct sockaddr_in*)&skaddr)->sin_port;
            }
            peer->pid = PID();
            peer->conn_flag = TRUE;

            sprintf (strid, "%lu", peer->pid());
            DC_dict_add_object_with_key (&serv->conn_peers, (char*)strid, (void*)peer);
            break;
        case JCOM_DISCONN:
            sprintf (strid, "%lu", hdr->PID);
            peer = DC_dict_get_object_with_key (&serv->conn_peers, (char*)strid);
            if (peer) {
                DC_dict_remove_object_with_key (&serv->conn_peers, (char*)strid);
                memset (peer, '\0', sizeof (jPeer));
                serv->mem_peers.release_peer (&serv->mem_peers, peer);
            }
            break;
        case JCOM_TRANS:
            sprintf (strid, "%lu", hdr->PID);
            if (hdr->length > max_trans) {
                return 0;
            }
            peer = DC_dict_get_object_with_key (&serv->peers, (char*)strid);
            if (peer) {
                peer->netbuf = netbuf; 
            }
            break;
        default:
            break;
    }

    if (serv->handler) {
        serv->handler (hdr->command, peer, serv->data);
    }
    if (netbuf) {
        serv->mem_netbuf.free_buffer (&serv->mem_netbuf, netbuf);
    }

    return 0;
}
int ServerRun (jServer *serv, void (*core)(void*, jPeer*), void *data, int tm_wait, unsigned int max_trans)
{
    fd_t fds;
    int ret;
    struct timeval timew;
    int max_fd,i;
    jPeer *peer;
    int csock;
    void *save_ptr;
    unsigned int ping_count = 0;

    FD_ZERO (&fds);
    FD_SET (serv->inet_info.sock_fd, &fds);
    max_fd = serv->inet_info.sock_fd;

    peer = serv->mem_peers.new_peer (&serv->mem_peers);
    peer->inet_info.sock_fd = serv->inet_info.sock_fd;
    peer->inet_info.proto   = serv->inet_info.proto;
    peer->inet_info.port    = serv->inet_info.port;
    DC_list_add_object (&serv->sock_peers, (void*)peer);

    do {
        if (tm_wait) {
            timew.tv_sec = tm_wait;
            timew.tv_usec = 0;
        }

        ret = select (max_fd+1, &fds, NULL, NULL, tm_wait?:&timew:NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        } else if (ret == 0) {
            if (core) {
                ret = core (data, NULL);
                if (ret) return 0;
            }
            ping_count++;
        } else {
            save_ptr = NULL;
            while ((peer = (jPeer*)DC_list_next_object (&serv->sock_peers, &save_ptr)) && ret) {
                if (FD_ISSET (peer->inet_info.sock_fd, &fds)) {
                    if (peer->inet_info.sock_fd == serv->inet_info.sock_fd) {
                        if (serv->inet_info.proto == PROTO_TCP) {
                            csock = accept (serv->inet_info.sock_fd, NULL, NULL);
                            if (csock > 0) {
                                peer = serv->mem_peers.new_peer (&serv->mem_peers);
                                if (peer) {
                                    memset (peer, '\0', sizeof (jPeer));
                                    peer->inet_info.sock_fd = csock;
                                    peer->inet_info.proto   = serv->inet_info.proto;
                                    peer->conn_time         = ping_count;
                                    DC_list_add_object (&serv->sock_peers, (void*)peer);
                                    max_fd = (max_fd>csock?max_fd:csock);
                                    FD_SET (csock, &fds);
                                } else {
                                    close (csock);
                                }                            
                            }
                        } else if (serv->inet_info.proto == PROTO_UDP) {
                            DoRequest (peer, serv, max_trans);
                        }
                    } else {
                        DoRequest (peer, serv, max_trans);
                    }

                    ret--;
                }
            }
        }
    } while (1);
}

extern void ServerClose (jServer *serv);

